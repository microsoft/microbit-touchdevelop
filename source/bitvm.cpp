#include "BitVM.h"
#include "MicroBitTouchDevelop.h"
#include <cstdlib>
#include <climits>
#include <cmath>
#include <vector>


#define DBG printf
//#define DBG(...)

#define bytecode ((uint16_t*)functionsAndBytecode)

#define getstr(off) ((const char*)&bytecode[off])
#define getbytes(off) ((const uint8_t*)&bytecode[off])

// Macros to reference function pointer in the jump-list
// c in mbitc - stands for 'common'
#define mbit(x) (void*)bitvm_micro_bit::x,
#define mbitc(x) (void*)micro_bit::x,


namespace bitvm {

  uint32_t ldloc(RefLocal *r)
  {
    //DBG("LD "); r->print();
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
    //DBG("ST "); r->print();
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

  uint32_t ldfld(RefRecord *r, int idx)
  {
    return r->ld(idx);
  }

  uint32_t ldfldRef(RefRecord *r, int idx)
  {
    return r->ldref(idx);
  }

  void stfld(RefRecord *r, int idx, uint32_t val)
  {
    r->st(idx, val);
  }

  void stfldRef(RefRecord *r, int idx, uint32_t val)
  {
    r->stref(idx, val);
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

    RefString *to_character(int x)
    {
      return new RefString(ManagedString((char)x));
    }

    RefString *to_string(int x)
    {
      return new RefString(ManagedString(x));
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
    RefString *dempty;

    RefString *mkEmpty()
    {
      if (dempty == NULL) {
        dempty = new RefString("");
        dempty->canLeak();
      }

      dempty->ref();
      return dempty;
    }

    RefString *fromLiteral(const char *p)
    {
      if (!*p) return mkEmpty();
      RefString *res = new RefString(p);
      res->ref(); // never unrefed
      res->canLeak();
      return res;
    }

    RefString *concat(RefString *s1, RefString *s2) {
      return new RefString(s1->v + s2->v);
    }

    RefString *concat_op(RefString *s1, RefString *s2) {
      return concat(s1, s2);
    }

    RefString *substring(RefString *s, int i, int j) {
      return new RefString(s->v.substring(i, j));
    }

    bool equals(RefString *s1, RefString *s2) {
      return s1->v == s2->v;
    }

    int count(RefString *s) {
      return s->length();
    }

    RefString *at(RefString *s, int i) {
      return new RefString(ManagedString((char)s->v.charAt(i)));
    }

    int to_character_code(RefString *s) {
      return s->length() > 0 ? s->charAt(0) : '\0';
    }

    int code_at(RefString *s, int i) {
      return i < s->length() && i >= 0 ? s->charAt(i) : '\0';
    }

    int to_number(RefString *s) {
      return atoi(s->toCharArray());
    }

    void post_to_wall(RefString *s) { printf("%s\n", s->toCharArray()); }
  }

  namespace bitvm_boolean {
    // Cache the string literals "true" and "false" when used.
    // Note that the representation of booleans stays the usual C-one.
    RefString *sTrue, *sFalse;
    RefString *to_string(int v)
    {
      if (v) {
        if (sTrue == NULL) sTrue = string::fromLiteral("true");
        sTrue->ref();
        return sTrue;
      } else {
        if (sFalse == NULL) sFalse = string::fromLiteral("false");
        sFalse->ref();
        return sFalse;
      }            
    }
  }

  // The global array strings[] maps string-literal-id (as determined by the
  // code generator) to the actual string literal, which is a RefString* pointer. 
  // It is populated lazily. 
  uint32_t stringLiteral(int id, uint32_t off)
  {
    uint32_t tmp = strings[id];
    if (tmp == 0) {
      tmp = strings[id] = (uint32_t)string::fromLiteral(getstr(off));
    }
    incr(tmp);
    return tmp;
  }


  namespace collection {

    RefCollection *mk()
    {
      RefCollection *r = new RefCollection();
      return r;
    }

    int count(RefCollection *c) { return c->data.size(); }
    void add(RefCollection *c, int x) { c->data.push_back(x); }

    inline bool in_range(RefCollection *c, int x) {
      return (0 <= x && x < (int)c->data.size());
    }

    int at(RefCollection *c, int x) {
      if (in_range(c, x))
        return c->data.at(x);
      else {
        error(ERR_OUT_OF_BOUNDS);
        return 0;
      }
    }

    void remove_at(RefCollection *c, int x) {
      if (!in_range(c, x))
        return;

      c->data.erase(c->data.begin()+x);
    }

    void set_at(RefCollection *c, int x, int y) {
      if (!in_range(c, x))
        return;

      c->data.at(x) = y;
    }

    int index_of(RefCollection *c, uint32_t x, int start) {
      if (!in_range(c, start))
        return -1;

      for (uint32_t i = start; i < c->data.size(); ++i)
        if (c->data.at(i) == x)
          return i;

      return -1;
    }

    int remove(RefCollection *c, int x) {
      int idx = index_of(c, x, 0);
      if (idx >= 0) {
        remove_at(c, idx);
        return 1;
      }

      return 0;
    }
  }

  namespace refcollection {

    RefRefCollection *mk()
    {
      RefRefCollection *r = new RefRefCollection();
      return r;
    }

    int count(RefRefCollection *c) { return c->data.size(); }

    void add(RefRefCollection *c, RefObject *x) {
      if (x) x->ref();
      c->data.push_back(x);
    }

    inline bool in_range(RefRefCollection *c, int x) {
      return (0 <= x && x < (int)c->data.size());
    }

    RefObject *at(RefRefCollection *c, int x) {
      if (in_range(c, x)) {
        RefObject *tmp = c->data.at(x);
        if (tmp) tmp->ref();
        return tmp;
      }
      else {
        error(ERR_OUT_OF_BOUNDS);
        return 0;
      }
    }

    void remove_at(RefRefCollection *c, int x) {
      if (!in_range(c, x))
        return;

      RefObject *tmp = c->data.at(x);
      if (tmp) tmp->unref();
      c->data.erase(c->data.begin()+x);
    }

    void set_at(RefRefCollection *c, int x, RefObject *y) {
      if (!in_range(c, x))
        return;

      RefObject *tmp = c->data.at(x);
      if (tmp) tmp->unref();
      if (y) y->ref();
      c->data.at(x) = y;
    }

    int index_of(RefRefCollection *c, RefObject *x, int start) {
      if (!in_range(c, start))
        return -1;

      for (uint32_t i = start; i < c->data.size(); ++i)
        if (c->data.at(i)->equals(x))
          return i;

      return -1;
    }

    int remove(RefRefCollection *c, RefObject *x) {
      int idx = index_of(c, x, 0);
      if (idx >= 0) {
        remove_at(c, idx);
        return 1;
      }

      return 0;
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

  namespace action {
    RefAction* mk(int reflen, int totallen, int startptr)
    {
      check(0 <= reflen && reflen <= totallen, ERR_SIZE, 1);
      check(reflen <= totallen && totallen <= 255, ERR_SIZE, 2);

      void *ptr = ::operator new(sizeof(RefAction) + totallen * sizeof(uint32_t));
      RefAction *r = new (ptr) RefAction();
      r->len = totallen;
      r->reflen = reflen;
      uint32_t tmp = (uint32_t)&bytecode[startptr];
      r->func = (ActionCB)(tmp | 1);
      memset(r->fields, 0, r->len * sizeof(uint32_t));

      return r;
    }

    void run(RefAction *a)
    {
      //DBG("run "); a->print();
      a->run();
    }
  }


  // ---------------------------------------------------------------------------
  // Implementation of the BBC micro:bit features
  // ---------------------------------------------------------------------------
  using namespace touch_develop;

  namespace bitvm_micro_bit {

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    typedef RefAction *Action;

    void callback(MicroBitEvent e, Action a) {
      a->run();
    }


    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    void onPinPressed(int pin, Action a) {
      if (a != NULL) {
        a->ref();
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
        uBit.MessageBus.ignore(
          pin,
          MICROBIT_BUTTON_EVT_CLICK,
          (void (*)(MicroBitEvent, void*)) callback);
        uBit.MessageBus.listen(
          pin,
          MICROBIT_BUTTON_EVT_CLICK,
          (void (*)(MicroBitEvent, void*)) callback,
          (void*) a);
      }
    }

    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------

    void onButtonPressedExt(int button, int event, Action a) {
      if (a != NULL) {
        a->ref();
        uBit.MessageBus.ignore(
          button,
          event,
          (void (*)(MicroBitEvent, void*)) callback);
        uBit.MessageBus.listen(
          button,
          event,
          (void (*)(MicroBitEvent, void*)) callback,
          (void*) a);
      }
    }

    void onButtonPressed(int button, Action a) {
      onButtonPressedExt(button, MICROBIT_BUTTON_EVT_CLICK, a);
    }


    // -------------------------------------------------------------------------
    // System
    // -------------------------------------------------------------------------
    
    void fiberHelper(void *a)
    {
      ((Action)a)->run();
    }

    void fiberDone(void *a)
    {
      ((Action)a)->unref();
      release_fiber();
    }


    void runInBackground(Action a) {
      if (a != NULL) {
        a->ref();
        create_fiber(fiberHelper, a, fiberDone);
      }
    }

    void forever_stub(void *a) {
      while (true) {
        ((Action)a)->run();
        micro_bit::pause(20);
      }
    }

    void forever(Action a) {
      if (a != NULL) {
        a->ref();
        create_fiber(forever_stub, a);
      }
    }

    // -------------------------------------------------------------------------
    // Images (helpers that create/modify a MicroBitImage)
    // -------------------------------------------------------------------------
    
    typedef RefStruct<MicroBitImage> RefImage;

    // Argument rewritten by the C++ emitter to be what we need
    RefImage *createImage(int w, int h, uint32_t bitmap) {
      return new RefImage(::touch_develop::micro_bit::createImage(w, h, getbytes(bitmap)));
    }

    RefImage *createImageFromString(RefString *s) {
      return new RefImage(::touch_develop::micro_bit::createImageFromString(s->v));
    }

    RefImage *displayScreenShot()
    {
      return new RefImage(uBit.display.screenShot());
    }

    void clearImage(RefImage *i) {
      ::touch_develop::micro_bit::clearImage(i->v);
    }

    int getImagePixel(RefImage *i, int x, int y) {
      return ::touch_develop::micro_bit::getImagePixel(i->v, x, y);
    }

    void setImagePixel(RefImage *i, int x, int y, int value) {
      ::touch_develop::micro_bit::setImagePixel(i->v, x, y, value);
    }

    int getImageWidth(RefImage *i) {
      return ::touch_develop::micro_bit::getImageWidth(i->v);
    }

    // -------------------------------------------------------------------------
    // Various "show"-style functions to display and scroll things on the screen
    // -------------------------------------------------------------------------

    void showLetter(RefString *s) {
      ::touch_develop::micro_bit::showLetter(s->v);
    }

    void scrollString(RefString *s, int delay) {
      ::touch_develop::micro_bit::scrollString(s->v, delay);
    }

    void showImage(RefImage *i, int offset) {
      ::touch_develop::micro_bit::showImage(i->v, offset);
    }

    void scrollImage(RefImage *i, int offset, int delay) {
      ::touch_develop::micro_bit::scrollImage(i->v, offset, delay);
    }

    void plotImage(RefImage *i, int offset) {
      ::touch_develop::micro_bit::plotImage(i->v, offset);
    }

    // These have their arguments rewritten by the C++ compiler.
    void showLeds(int w, int h, uint32_t bitmap, int delay) {
      ::touch_develop::micro_bit::showLeds(w, h, getbytes(bitmap), delay);
    }

    void plotLeds(int w, int h, uint32_t bitmap) {
      ::touch_develop::micro_bit::plotLeds(w, h, getbytes(bitmap));
    }

    void showAnimation(int w, int h, uint32_t bitmap, int ms) {
      ::touch_develop::micro_bit::showAnimation(w, h, getbytes(bitmap), ms);
    }

    // -------------------------------------------------------------------------
    // Functions that expose member fields of objects because the compilation 
    // scheme only works with the C-style calling convention 
    // -------------------------------------------------------------------------

    void compassCalibrateEnd() { uBit.compass.calibrateEnd(); }
    void compassCalibrateStart() { uBit.compass.calibrateStart(); }
    void reset() { uBit.reset(); }
    int thermometerGetTemperature() { return uBit.thermometer.getTemperature(); }

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

    void serialSendString(RefString *s)
    {
      uBit.serial.sendString(s->v);
    }

    RefString *serialReadString()
    {
      return new RefString(uBit.serial.readString());
    }
    
    void serialSendImage(RefImage *img)
    {
      uBit.serial.sendImage(img->v);
    }

    RefImage *serialReadImage(int width, int height)
    {
      return new RefImage(uBit.serial.readImage(width, height));
    }

    void serialSendDisplayState() { uBit.serial.sendDisplayState(); }
    void serialReadDisplayState() { uBit.serial.readDisplayState(); }
  }


  void error(ERROR code, int subcode)
  {
    printf("Error: %d [%d]\n", code, subcode);
    die();
  }


  uint32_t *globals;
  uint32_t *strings;
  int numGlobals, numStrings;

  uint32_t *allocate(uint16_t sz)
  {
    uint32_t *arr = new uint32_t[sz];
    memset(arr, 0, sz * 4);
    return arr;
  }

  void exec_binary(uint16_t *pc)
  {
    printf("start!\n");

    // XXX re-enable once the calibration code is fixed and [editor/embedded.ts]
    // properly prepends a call to [internal_main].
    // ::touch_develop::internal_main();

    uint32_t ver = *pc++;
    check(ver == V3BINARY, ERR_INVALID_BINARY_HEADER);
    numGlobals = *pc++;
    numStrings = *pc++;
    globals = allocate(numGlobals);
    strings = allocate(numStrings);
    pc += 3; // reserved

    uint32_t startptr = (uint32_t)pc;
    startptr |= 1; // Thumb state
    startptr = ((uint32_t (*)())startptr)();
    printf("stop main\n");

#ifdef DEBUG_MEMLEAKS
    bitvm::debugMemLeaks();
#endif

    return;
  }
  

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
  
  const uint32_t functionsAndBytecode[16000] __attribute__((aligned(0x20))) = {
    // Magic header to find it in the file
    0x08010801, 0x42424242, 0x08010801, 0x8de9d83e, 
    // List of pointers generated by scripts/functionTable.js
    #include "generated/pointers.inc"
  };


}

// vim: ts=2 sw=2 expandtab
