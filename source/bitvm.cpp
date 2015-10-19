#include "BitVM.h"
#include "MicroBitTouchDevelop.h"
#include <cstdlib>
#include <climits>
#include <cmath>
#include <vector>


#define DBG printf
//#define DBG(...)

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

  namespace contract
  {
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

  // This function is here, just so that the jump-tables for functions and enums
  // are always referenced and present in the hex file.
  void linkStuff()
  {
    uint32_t pc = bytecode[numStrings];
    pc += (int)enums[pc];
    pc += (int)functions[pc];
    if (pc < 100) linkStuff();
  }

  uint32_t *allocate(uint16_t sz)
  {
    uint32_t *arr = new uint32_t[sz];
    memset(arr, 0, sz * 4);
    return arr;
  }

  void exec_binary()
  {
    uint32_t pc = 0;
    uint32_t ver = bytecode[pc++];

    printf("start!\n");

    ::touch_develop::touch_develop::main();

    check(ver == V2BINARY, ERR_INVALID_BINARY_HEADER);
    numGlobals = bytecode[pc++];
    numStrings = bytecode[pc++];
    globals = allocate(numGlobals);
    strings = allocate(numStrings);
    pc += 3; // reserved

    // We need to make sure there is a code path where linkStuff() is executed
    // that the optimizing compiler cannot remove.
    // We know we cannot allocate that many globals, so it's safe to "call" it
    // under that assumption.
    if (numGlobals > 30000) {
      linkStuff();
    }

    uint32_t startptr = (uint32_t)&bytecode[pc];
    startptr |= 1; // Thumb state
    startptr = ((uint32_t (*)())startptr)();
    printf("stop main\n");

#ifdef DEBUG_MEMLEAKS
    bitvm::debugMemLeaks();
#endif

    return;
  }
  

  // The ARM Thumb generator in the JavaScript code is parsing
  // the hex file and looks for the two random numbers as present
  // in the header of this file.
  //
  // Then it fetches function pointer addresses from there.
  //
  // The way it's setup, it doesn't actually matter what the numbers
  // are (they are embedded in JSON metadata) provided they
  // are random enough not to occur elsewhere in the hex file.
  //
  // The comments PROC<n> and FUNC<n> refer to the procedures
  // and functions with given number of arguments.
  //
  // The generateEmbedInfo.js looks for them.
  //
  // The code generator will assert if the Touch Develop function
  // has different number of input/output parameters than the one
  // defined here.
  //
  // It of course should match the C++ implementation.

  void * const functions[] __attribute__((aligned(0x10))) = {
    (void*)0x684e35a0,
    (void*)0x7ebbb194,

    //-- PROC0
    mbitc(clearScreen)
    mbit(compassCalibrateEnd)
    mbit(compassCalibrateStart)
    mbit(reset)
    mbit(serialSendDisplayState)
    mbit(serialReadDisplayState)

    //-- PROC1
    (void*)bitvm_number::post_to_wall,
    (void*)string::post_to_wall,
    (void*)action::run,
    (void*)bitvm::incr,
    (void*)bitvm::decr,
    mbit(clearImage)
    mbitc(enablePitch)
    mbit(forever)
    mbitc(pause)
    mbit(runInBackground)
    mbitc(setBrightness)
    mbitc(showDigit)
    mbit(showLetter)
    mbit(serialSendString)
    mbit(serialSendImage)
    mbit(panic)

    //-- PROC2
    (void*)contract::assert,
    (void*)collection::add,
    (void*)collection::remove_at,
    (void*)refcollection::add,
    (void*)refcollection::remove_at,
    (void*)bitvm::stglb,
    (void*)bitvm::stglbRef,
    (void*)bitvm::stloc,
    (void*)bitvm::stlocRef,
    mbit(plotImage)
    mbitc(analogWritePin)
    mbitc(digitalWritePin)
    mbitc(i2c_write)
    mbit(onButtonPressed)
    mbit(onPinPressed)
    mbitc(pitch)
    mbitc(plot)
    mbit(scrollString)
    mbitc(scrollNumber)
    mbitc(setAnalogPeriodUs)
    mbit(showImage)
    mbitc(unPlot)

    //-- PROC3
    (void*)collection::set_at,
    (void*)refcollection::set_at,
    (void*)bitvm::stfld,
    (void*)bitvm::stfldRef,
    mbit(showLeds)
    mbitc(i2c_write2)
    mbit(onButtonPressedExt)
    mbit(scrollImage)
    
    //-- PROC4
    mbit(showAnimation)
    mbit(setImagePixel)

    //-- FUNC0
    (void*)string::mkEmpty,
    (void*)collection::mk,
    (void*)refcollection::mk,
    (void*)bitvm::const3,
    (void*)bitvm::mkloc,
    (void*)bitvm::mklocRef,
    mbitc(compassHeading)
    mbitc(getBrightness)
    mbitc(getCurrentTime)
    mbit(ioP0)
    mbit(ioP1)
    mbit(ioP2)
    mbit(ioP3)
    mbit(ioP4)
    mbit(ioP5)
    mbit(ioP6)
    mbit(ioP7)
    mbit(ioP8)
    mbit(ioP9)
    mbit(ioP10)
    mbit(ioP11)
    mbit(ioP12)
    mbit(ioP13)
    mbit(ioP14)
    mbit(ioP15)
    mbit(ioP16)
    mbit(ioP19)
    mbit(ioP20)
    mbit(serialReadString)

    //-- FUNC1
    (void*)boolean::not_,
    (void*)math::random,
    (void*)math::abs,
    (void*)math::sqrt,
    (void*)math::sign,
    (void*)string::count,
    (void*)string::to_character_code,
    (void*)string::to_number,
    (void*)bitvm_number::to_character,
    (void*)bitvm_number::to_string,
    (void*)collection::count,
    (void*)refcollection::count,
    (void*)bitvm_boolean::to_string,
    (void*)bitvm::ldglb,
    (void*)bitvm::ldglbRef,
    (void*)bitvm::ldloc,
    (void*)bitvm::ldlocRef,
    (void*)bitvm::is_invalid,
    mbitc(analogReadPin)
    mbitc(digitalReadPin)
    mbitc(getAcceleration)
    mbit(createImageFromString)
    mbit(getImageWidth)
    mbitc(i2c_read)
    mbitc(isButtonPressed)
    mbitc(isPinTouched)
    mbit(displayScreenShot)

    //-- FUNC2
    (void*)boolean::or_,
    (void*)boolean::and_,
    (void*)boolean::equals,
    (void*)bits::or_uint32,
    (void*)bits::and_uint32,
    (void*)bits::xor_uint32,
    (void*)bits::shift_left_uint32,
    (void*)bits::shift_right_uint32,
    (void*)bits::rotate_right_uint32,
    (void*)bits::rotate_left_uint32,
    (void*)number::lt,
    (void*)number::leq,
    (void*)number::neq,
    (void*)number::eq,
    (void*)number::gt,
    (void*)number::geq,
    (void*)number::plus,
    (void*)number::minus,
    (void*)number::div,
    (void*)number::times,
    (void*)math::max,
    (void*)math::min,
    (void*)math::mod,
    (void*)math::pow,
    (void*)string::concat_op,
    (void*)string::concat,
    (void*)string::equals,
    (void*)string::code_at,
    (void*)string::at,
    (void*)collection::at,
    (void*)collection::remove,
    (void*)refcollection::at,
    (void*)refcollection::remove,
    (void*)record::mk,
    (void*)bitvm::ldfld,
    (void*)bitvm::ldfldRef,
    (void*)bitvm::stringLiteral,
    mbitc(point)
    mbit(serialReadImage)

    //-- FUNC3
    (void*)math::clamp,
    (void*)string::substring,
    (void*)collection::index_of,
    (void*)refcollection::index_of,
    (void*)action::mk,
    (void*)bitvm::stclo,
    mbit(createImage)
    mbit(getImagePixel)
  };

  // This uses the same mechanism with the magic numbers as the
  // functions array above.
  const int enums[] __attribute__((aligned(0x10))) = {
    0x44f4ecc1,
    0x33e7fa08,
    ERR_INVALID_FUNCTION_HEADER,
    ERR_INVALID_BINARY_HEADER,
    ERR_STACK_ONRETURN,
    ERR_REF_DELETED,
    ERR_OUT_OF_BOUNDS,
    ERR_SIZE,
    MES_ALERT_EVT_ALARM1,
    MES_ALERT_EVT_ALARM2,
    MES_ALERT_EVT_ALARM3,
    MES_ALERT_EVT_ALARM4,
    MES_ALERT_EVT_ALARM5,
    MES_ALERT_EVT_ALARM6,
    MES_ALERT_EVT_DISPLAY_TOAST,
    MES_ALERT_EVT_FIND_MY_PHONE,
    MES_ALERT_EVT_PLAY_RINGTONE,
    MES_ALERT_EVT_PLAY_SOUND,
    MES_ALERT_EVT_VIBRATE,
    MES_AUDIO_RECORDER_EVT_LAUNCH,
    MES_AUDIO_RECORDER_EVT_START_CAPTURE,
    MES_AUDIO_RECORDER_EVT_STOP_CAPTURE,
    MES_AUDIO_RECORDER_EVT_STOP,
    MES_CAMERA_EVT_LAUNCH_PHOTO_MODE,
    MES_CAMERA_EVT_LAUNCH_VIDEO_MODE,
    MES_CAMERA_EVT_START_VIDEO_CAPTURE,
    MES_CAMERA_EVT_STOP_PHOTO_MODE,
    MES_CAMERA_EVT_STOP_VIDEO_CAPTURE,
    MES_CAMERA_EVT_STOP_VIDEO_MODE,
    MES_CAMERA_EVT_TAKE_PHOTO,
    MES_CAMERA_EVT_TOGGLE_FRONT_REAR,
    MES_DEVICE_INFO_ID,
    MES_PLAY_CONTROLLER_ID,
    MES_REMOTE_CONTROL_EVT_FORWARD,
    MES_REMOTE_CONTROL_EVT_NEXTTRACK,
    MES_REMOTE_CONTROL_EVT_PAUSE,
    MES_REMOTE_CONTROL_EVT_PLAY,
    MES_REMOTE_CONTROL_EVT_PREVTRACK,
    MES_REMOTE_CONTROL_EVT_REWIND,
    MES_REMOTE_CONTROL_EVT_STOP,
    MES_REMOTE_CONTROL_EVT_VOLUMEDOWN,
    MES_REMOTE_CONTROL_EVT_VOLUMEUP,
    MES_SIGNAL_STRENGTH_ID,
    MICROBIT_ID_BUTTON_A,
    MICROBIT_ID_BUTTON_AB,
    MICROBIT_ID_BUTTON_B,
    MICROBIT_ID_IO_P0,
    MICROBIT_ID_IO_P1,
    MICROBIT_ID_IO_P2,
  };

}

// vim: ts=2 sw=2 expandtab
