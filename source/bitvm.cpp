#include "BitVM.h"
#include "MicroBitTouchDevelop.h"
#include <cstdlib>
#include <climits>
#include <cmath>
#include <vector>


#define DBG printf
//#define DBG(...)

#define getstr(off) ((const char*)&bytecode[off])
#define getbytes(off) ((ImageData*)(void*)&bytecode[off])

// Macros to reference function pointer in the jump-list
// c in mbitc - stands for 'common'
#define mbit(x) (void*)bitvm_micro_bit::x,
#define mbitc(x) (void*)micro_bit::x,

namespace bitvm {
  uint16_t *bytecode;

  uint32_t ldloc(RefLocal *r)
  {
    return r->v;
  }

  uint32_t ldlocRef(RefRefLocal *r)
  {
    uint32_t tmp = r->v;
    incr(tmp);
    return tmp;
  }

  void stloc(RefLocal *r, uint32_t v)
  {
    r->v = v;
  }

  void stlocRef(RefRefLocal *r, uint32_t v)
  {
    decr(r->v);
    r->v = v;
  }

  RefLocal *mkloc()
  {
    return new RefLocal();
  }

  RefRefLocal *mklocRef()
  {
    return new RefRefLocal();
  }

  // All of the functions below unref() self. This is for performance reasons -
  // the code emitter will not emit the unrefs for them.
  
  uint32_t ldfld(RefRecord *r, int idx)
  {
    auto tmp = r->ld(idx);
    r->unref();
    return tmp;
  }

  uint32_t ldfldRef(RefRecord *r, int idx)
  {
    auto tmp = r->ldref(idx);
    r->unref();
    return tmp;
  }

  void stfld(RefRecord *r, int idx, uint32_t val)
  {
    r->st(idx, val);
    r->unref();
  }

  void stfldRef(RefRecord *r, int idx, uint32_t val)
  {
    r->stref(idx, val);
    r->unref();
  }

  uint32_t ldglb(int idx)
  {
    check(0 <= idx && idx < numGlobals, ERR_OUT_OF_BOUNDS, 7);
    return globals[idx];
  }

  uint32_t ldglbRef(int idx)
  {
    check(0 <= idx && idx < numGlobals, ERR_OUT_OF_BOUNDS, 7);
    uint32_t tmp = globals[idx];
    incr(tmp);
    return tmp;
  }

  uint32_t is_invalid(uint32_t v)
  {
    return v == 0;
  }

  // note the idx comes last - it's more convenient that way in the emitter
  void stglb(uint32_t v, int idx)
  {
    check(0 <= idx && idx < numGlobals, ERR_OUT_OF_BOUNDS, 7);
    globals[idx] = v;
  }

  void stglbRef(uint32_t v, int idx)
  {
    check(0 <= idx && idx < numGlobals, ERR_OUT_OF_BOUNDS, 7);
    decr(globals[idx]);
    globals[idx] = v;
  }

  // Store a captured local in a closure. It returns the action, so it can be chained.
  RefAction *stclo(RefAction *a, int idx, uint32_t v)
  {
    //DBG("STCLO "); a->print(); DBG("@%d = %p\n", idx, (void*)v);
    a->st(idx, v);
    return a;
  }

  // This one is used for testing in 'bitvm test0'
  uint32_t const3() { return 3; }

#ifdef DEBUG_MEMLEAKS
  std::set<RefObject*> allptrs;
  void debugMemLeaks()
  {
    printf("LIVE POINTERS:\n");
    for(std::set<RefObject*>::iterator itr = allptrs.begin();itr!=allptrs.end();itr++)
    {
      (*itr)->print();
    }    
    printf("\n");
  }
#else
  void debugMemLeaks() {}
#endif

  namespace bitvm_number {
    void post_to_wall(int n) { printf("%d\n", n); }

    StringData *to_character(int x)
    {
      return ManagedString((char)x).leakData();
    }

    StringData *to_string(int x)
    {
      return ManagedString(x).leakData();
    }
  }

  namespace contract {
    void assert(int cond, uint32_t msg)
    {
      if (cond == 0) {
        printf("Assertion failed: %s\n", getstr(msg));
        die();
      }
    }
  }

  namespace string {
    StringData *mkEmpty()
    {
      return ManagedString::EmptyString.leakData();
    }

    StringData *concat(StringData *s1, StringData *s2) {
      ManagedString a(s1), b(s2);
      return (a + b).leakData();
    }

    StringData *concat_op(StringData *s1, StringData *s2) {
      return concat(s1, s2);
    }

    StringData *substring(StringData *s, int i, int j) {
      return (ManagedString(s).substring(i, j)).leakData();
    }

    bool equals(StringData *s1, StringData *s2) {
      return ManagedString(s1) == ManagedString(s2);
    }

    int count(StringData *s) {
      return s->len;
    }

    StringData *at(StringData *s, int i) {
      return ManagedString((char)ManagedString(s).charAt(i)).leakData();
    }

    int to_character_code(StringData *s) {
      return ManagedString(s).charAt(0);
    }

    int code_at(StringData *s, int i) {
      return ManagedString(s).charAt(i);
    }

    int to_number(StringData *s) {
      return atoi(s->data);
    }

    void post_to_wall(StringData *s) { printf("%s\n", s->data); }
  }

  namespace bitvm_boolean {
    // Cache the string literals "true" and "false" when used.
    // Note that the representation of booleans stays the usual C-one.
    
    static const char sTrue[]  __attribute__ ((aligned (4))) = "\xff\xff\x04\x00" "true\0";
    static const char sFalse[] __attribute__ ((aligned (4))) = "\xff\xff\x05\x00" "false\0";

    StringData *to_string(int v)
    {
      if (v) {
        return (StringData*)(void*)sTrue;
      } else {
        return (StringData*)(void*)sFalse;
      }            
    }
  }

  StringData *mkStringData(uint32_t len)
  {
    StringData *r = (StringData*)malloc(sizeof(StringData)+len+1);
    r->init();
    r->len = len;
    memset(r->data, '\0', len + 1);
    return r;
  }

  // The proper StringData* representation is already laid out in memory by the code generator.
  uint32_t stringData(uint32_t lit)
  {
    return (uint32_t)getstr(lit);
  }


  namespace collection {

    RefCollection *mk(uint32_t flags)
    {
      RefCollection *r = new RefCollection(flags);
      return r;
    }

    int count(RefCollection *c) { return c->data.size(); }

    void add(RefCollection *c, uint32_t x) {
      if (c->flags & 1) incr(x);
      c->data.push_back(x);
    }

    inline bool in_range(RefCollection *c, int x) {
      return (0 <= x && x < (int)c->data.size());
    }

    uint32_t at(RefCollection *c, int x) {
      if (in_range(c, x)) {
        uint32_t tmp = c->data.at(x);
        if (c->flags & 1) incr(tmp);
        return tmp;
      }
      else {
        error(ERR_OUT_OF_BOUNDS);
        return 0;
      }
    }

    void remove_at(RefCollection *c, int x) {
      if (!in_range(c, x))
        return;

      if (c->flags & 1) decr(c->data.at(x));
      c->data.erase(c->data.begin()+x);
    }

    void set_at(RefCollection *c, int x, uint32_t y) {
      if (!in_range(c, x))
        return;

      if (c->flags & 1) {
        decr(c->data.at(x));
        incr(y);
      }
      c->data.at(x) = y;
    }

    int index_of(RefCollection *c, uint32_t x, int start) {
      if (!in_range(c, start))
        return -1;

      if (c->flags & 2) {
        StringData *xx = (StringData*)x;
        for (uint32_t i = start; i < c->data.size(); ++i) {
          StringData *ee = (StringData*)c->data.at(i);
          if (xx->len == ee->len && memcmp(xx->data, ee->data, xx->len) == 0)
            return (int)i;
        }
      } else {
        for (uint32_t i = start; i < c->data.size(); ++i)
          if (c->data.at(i) == x)
            return (int)i;
      }

      return -1;
    }

    int remove(RefCollection *c, uint32_t x) {
      int idx = index_of(c, x, 0);
      if (idx >= 0) {
        remove_at(c, idx);
        return 1;
      }

      return 0;
    }
  }

  namespace buffer {

    RefBuffer *mk(uint32_t size)
    {
      RefBuffer *r = new RefBuffer();
      r->data.resize(size);
      return r;
    }

    char *cptr(RefBuffer *c)
    {
      return (char*)&c->data[0];
    }

    int count(RefBuffer *c) { return c->data.size(); }

    void fill(RefBuffer *c, int v)
    {
      memset(cptr(c), v, count(c));
    }

    void fill_random(RefBuffer *c)
    {
      int len = count(c);
      for (int i = 0; i < len; ++i)
        c->data[i] = uBit.random(0x100);
    }

    void add(RefBuffer *c, uint32_t x) {
      c->data.push_back(x);
    }

    inline bool in_range(RefBuffer *c, int x) {
      return (0 <= x && x < (int)c->data.size());
    }

    uint32_t at(RefBuffer *c, int x) {
      if (in_range(c, x)) {
        return c->data[x];
      }
      else {
        error(ERR_OUT_OF_BOUNDS);
        return 0;
      }
    }

    void set(RefBuffer *c, int x, uint32_t y) {
      if (!in_range(c, x))
        return;
      c->data[x] = y;
    }
  }

  namespace bitvm_bits {
    RefBuffer *create_buffer(int size)
    {
      return buffer::mk(size);
    }
  }

  namespace record {
    RefRecord* mk(int reflen, int totallen)
    {
      check(0 <= reflen && reflen <= totallen, ERR_SIZE, 1);
      check(reflen <= totallen && totallen <= 255, ERR_SIZE, 2);

      void *ptr = ::operator new(sizeof(RefRecord) + totallen * sizeof(uint32_t));
      RefRecord *r = new (ptr) RefRecord();
      r->len = totallen;
      r->reflen = reflen;
      memset(r->fields, 0, r->len * sizeof(uint32_t));
      return r;
    }
  }

  typedef uint32_t Action;

  namespace action {
    Action mk(int reflen, int totallen, int startptr)
    {
      check(0 <= reflen && reflen <= totallen, ERR_SIZE, 1);
      check(reflen <= totallen && totallen <= 255, ERR_SIZE, 2);
      check(bytecode[startptr] == 0xffff, ERR_INVALID_BINARY_HEADER, 3);
      check(bytecode[startptr + 1] == 0, ERR_INVALID_BINARY_HEADER, 4);


      uint32_t tmp = (uint32_t)&bytecode[startptr];

      if (totallen == 0) {
        return tmp; // no closure needed
      }

      void *ptr = ::operator new(sizeof(RefAction) + totallen * sizeof(uint32_t));
      RefAction *r = new (ptr) RefAction();
      r->len = totallen;
      r->reflen = reflen;
      r->func = (ActionCB)((tmp + 4) | 1);
      memset(r->fields, 0, r->len * sizeof(uint32_t));

      return (Action)r;
    }

    void run1(Action a, int arg)
    {
      if (hasVTable(a))
        ((RefAction*)a)->run(arg);
      else {
        check(*(uint16_t*)a == 0xffff, ERR_INVALID_BINARY_HEADER, 4);
        ((ActionCB)((a + 4) | 1))(NULL, NULL, arg);
      }
    }

    void run(Action a)
    {
      action::run1(a, 0);
    }
  }


  // ---------------------------------------------------------------------------
  // Implementation of the BBC micro:bit features
  // ---------------------------------------------------------------------------
  using namespace touch_develop;

  namespace bitvm_micro_bit {

    // ---------------------------------------------------------------------------
    // An adapter for the API expected by the run-time.
    // ---------------------------------------------------------------------------

    map<pair<int, int>, Action> handlersMap;

    // We have the invariant that if [dispatchEvent] is registered against the DAL
    // for a given event, then [handlersMap] contains a valid entry for that
    // event.
    void dispatchEvent(MicroBitEvent e) {
      Action curr = handlersMap[{ e.source, e.value }];
      if (curr)
        action::run(curr);

      curr = handlersMap[{ e.source, MICROBIT_EVT_ANY }];
      if (curr)
        action::run1(curr, e.value);
    }

    void registerWithDal(int id, int event, Action a) {
      Action prev = handlersMap[{ id, event }];
      if (prev)
        decr(prev);
      else
        uBit.MessageBus.listen(id, event, dispatchEvent);
      incr(a);
      handlersMap[{ id, event }] = a;
    }

    void on_event(int id, Action a) {
      if (a != 0) {
        registerWithDal(id, MICROBIT_EVT_ANY, a);
      }
    }

    void onDeviceInfo(int event, Action a) {
      if (a != 0) {
        registerWithDal(MES_DEVICE_INFO_ID, event, a);
      }
    }

    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    void onPinPressed(int pin, Action a) {
      if (a != 0) {
        // Forces the PIN to switch to makey-makey style detection.
        switch(pin) {
          case MICROBIT_ID_IO_P0:
            uBit.io.P0.isTouched();
            break;
          case MICROBIT_ID_IO_P1:
            uBit.io.P1.isTouched();
            break;
          case MICROBIT_ID_IO_P2:
            uBit.io.P2.isTouched();
            break;
        }
        registerWithDal(pin, MICROBIT_BUTTON_EVT_CLICK, a);
      }
    }

    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------

    void onButtonPressedExt(int button, int event, Action a) {
      if (a != 0) {
        registerWithDal(button, event, a);
      }
    }

    void onButtonPressed(int button, Action a) {
      onButtonPressedExt(button, MICROBIT_BUTTON_EVT_CLICK, a);
    }


    // -------------------------------------------------------------------------
    // System
    // -------------------------------------------------------------------------
    
    void fiberDone(void *a)
    {
      decr((Action)a);
      release_fiber();
    }


    void runInBackground(Action a) {
      if (a != 0) {
        incr(a);
        create_fiber((void(*)(void*))action::run, (void*)a, fiberDone);
      }
    }

    void forever_stub(void *a) {
      while (true) {
        action::run((Action)a);
        micro_bit::pause(20);
      }
    }

    void forever(Action a) {
      if (a != 0) {
        incr(a);
        create_fiber(forever_stub, (void*)a);
      }
    }

    // -------------------------------------------------------------------------
    // Images (helpers that create/modify a MicroBitImage)
    // -------------------------------------------------------------------------
    
    // Argument rewritten by the code emitter to be what we need
    ImageData *createImage(uint32_t lit) {
      return MicroBitImage(getbytes(lit)).clone().leakData();
    }

    ImageData *createReadOnlyImage(uint32_t lit) {
      return getbytes(lit);
    }

    ImageData *createImageFromString(StringData *s) {
      return ::touch_develop::micro_bit::createImageFromString(ManagedString(s)).leakData();
    }

    ImageData *displayScreenShot()
    {
      return uBit.display.screenShot().leakData();
    }
    
    ImageData *imageClone(ImageData *i)
    {
      return MicroBitImage(i).clone().leakData();
    }

    void clearImage(ImageData *i) {
      MicroBitImage(i).clear();
    }

    int getImagePixel(ImageData *i, int x, int y) {
      int pix = MicroBitImage(i).getPixelValue(x, y);
      if (pix < 0) return 0;
      return pix;
    }

    void setImagePixel(ImageData *i, int x, int y, int value) {
      MicroBitImage(i).setPixelValue(x, y, value);
    }

    int getImageHeight(ImageData *i) {
      return i->height;
    }

    int getImageWidth(ImageData *i) {
      return i->width;
    }

    bool isImageReadOnly(ImageData *i) {
      return i->isReadOnly();
    }

    // -------------------------------------------------------------------------
    // Various "show"-style functions to display and scroll things on the screen
    // -------------------------------------------------------------------------

    void showLetter(StringData *s) {
      ::touch_develop::micro_bit::showLetter(ManagedString(s));
    }

    void scrollString(StringData *s, int delay) {
      ::touch_develop::micro_bit::scrollString(ManagedString(s), delay);
    }

    void showImage(ImageData *i, int offset) {
      ::touch_develop::micro_bit::showImage(MicroBitImage(i), offset);
    }

    void scrollImage(ImageData *i, int offset, int delay) {
      ::touch_develop::micro_bit::scrollImage(MicroBitImage(i), offset, delay);
    }

    void plotImage(ImageData *i, int offset) {
      ::touch_develop::micro_bit::plotImage(MicroBitImage(i), offset);
    }

    // [lit] argument is rewritted by code emitted
    void showLeds(uint32_t lit, int delay) {
      uBit.display.print(MicroBitImage(getbytes(lit)), 0, 0, 0, delay);
    }

    void plotLeds(uint32_t lit) {
      plotImage(getbytes(lit), 0);
    }

    void showAnimation(uint32_t lit, int ms) {
      uBit.display.animate(MicroBitImage(getbytes(lit)), ms, 5, 0);
    }

    // -------------------------------------------------------------------------
    // Functions that expose member fields of objects because the compilation 
    // scheme only works with the C-style calling convention 
    // -------------------------------------------------------------------------

    void reset() { uBit.reset(); }
    int thermometerGetTemperature() { return uBit.thermometer.getTemperature(); }

    void displayStopAnimation() {
      uBit.display.stopAnimation();
    }

    MicroBitPin *ioP0() { return &uBit.io.P0; }
    MicroBitPin *ioP1() { return &uBit.io.P1; }
    MicroBitPin *ioP2() { return &uBit.io.P2; }
    MicroBitPin *ioP3() { return &uBit.io.P3; }
    MicroBitPin *ioP4() { return &uBit.io.P4; }
    MicroBitPin *ioP5() { return &uBit.io.P5; }
    MicroBitPin *ioP6() { return &uBit.io.P6; }
    MicroBitPin *ioP7() { return &uBit.io.P7; }
    MicroBitPin *ioP8() { return &uBit.io.P8; }
    MicroBitPin *ioP9() { return &uBit.io.P9; }
    MicroBitPin *ioP10() { return &uBit.io.P10; }
    MicroBitPin *ioP11() { return &uBit.io.P11; }
    MicroBitPin *ioP12() { return &uBit.io.P12; }
    MicroBitPin *ioP13() { return &uBit.io.P13; }
    MicroBitPin *ioP14() { return &uBit.io.P14; }
    MicroBitPin *ioP15() { return &uBit.io.P15; }
    MicroBitPin *ioP16() { return &uBit.io.P16; }
    MicroBitPin *ioP19() { return &uBit.io.P19; }
    MicroBitPin *ioP20() { return &uBit.io.P20; }

    void panic(int code)
    {
      uBit.panic(code);
    }

    void serialSendString(StringData *s)
    {
      uBit.serial.sendString(ManagedString(s));
    }

    StringData *serialReadString()
    {
      return uBit.serial.readString().leakData();
    }
    
    void serialSendImage(ImageData *img)
    {
      uBit.serial.sendImage(MicroBitImage(img));
    }

    ImageData *serialReadImage(int width, int height)
    {
      return uBit.serial.readImage(width, height).leakData();
    }

    void serialSendDisplayState() { uBit.serial.sendDisplayState(); }
    void serialReadDisplayState() { uBit.serial.readDisplayState(); }

    void i2cReadBuffer(int address, RefBuffer *buf)
    {
      uBit.i2c.read(address << 1, buffer::cptr(buf), buffer::count(buf));
    }

    void i2cWriteBuffer(int address, RefBuffer *buf)
    {
      uBit.i2c.write(address << 1, buffer::cptr(buf), buffer::count(buf));
    }

    int i2cReadRaw(int address, char *data, int length, int repeated)
    {
      return uBit.i2c.read(address, data, length, repeated);
    }

    int i2cWriteRaw(int address, const char *data, int length, int repeated)
    {
      return uBit.i2c.write(address, data, length, repeated);
    }
  }


  void error(ERROR code, int subcode)
  {
    printf("Error: %d [%d]\n", code, subcode);
    die();
  }


  uint32_t *globals;
  int numGlobals;

  uint32_t *allocate(uint16_t sz)
  {
    uint32_t *arr = new uint32_t[sz];
    memset(arr, 0, sz * 4);
    return arr;
  }

  void checkStr(bool cond, const char *msg)
  {
    if (!cond) {
      while (true) {
        micro_bit::scrollString(msg, 100);
        micro_bit::pause(100);
      }
    }
  }

  int templateHash()
  {
    return ((int*)bytecode)[4];
  }

  int programHash()
  {
    return ((int*)bytecode)[6];
  }

  void exec_binary(uint16_t *pc)
  {
    // XXX re-enable once the calibration code is fixed and [editor/embedded.ts]
    // properly prepends a call to [internal_main].
    // ::touch_develop::internal_main();
    
    uint32_t ver = *pc++;
    checkStr(ver == 0x4207, ":( Bad runtime version");
    numGlobals = *pc++;
    globals = allocate(numGlobals);

    bytecode = *((uint16_t**)pc);  // the actual bytecode is here
    pc += 2;

    // just compare the first word
    checkStr(((uint32_t*)bytecode)[0] == 0x923B8E70 &&
             templateHash() == ((int*)pc)[0],
             ":( Failed partial flash");

    uint32_t startptr = (uint32_t)bytecode;
    startptr += 48; // header
    startptr |= 1; // Thumb state

    ((uint32_t (*)())startptr)();

#ifdef DEBUG_MEMLEAKS
    bitvm::debugMemLeaks();
#endif

    return;
  }
}  

using namespace bitvm;

#include "generated/extensions.inc"

namespace bitvm {
  // The ARM Thumb generator in the JavaScript code is parsing
  // the hex file and looks for the magic numbers as present here.
  //
  // Then it fetches function pointer addresses from there.
  //
  // The code generator will assert if the Touch Develop function
  // has different number of input/output parameters than the one
  // defined here.
  //
  // It of course should match the C++ implementation.
  
  const uint32_t functionsAndBytecode[] __attribute__((aligned(0x20))) = {
    // Magic header to find it in the file
    0x08010801, 0x42424242, 0x08010801, 0x8de9d83e, 
    // List of pointers generated by scripts/functionTable.js
    #include "generated/pointers.inc"
    #include "generated/extpointers.inc"
  };


}

// vim: ts=2 sw=2 expandtab
