#include "BitVM.h"
#include "MicroBitTouchDevelop.h"
#include <stdlib.h>
#include <climits>
#include <cmath>
#include <vector>


#define DBG printf
//#define DBG(...)


namespace bitvm {
#define getstr(off) ((const char*)&bytecode[off])
#define getbytes(off) ((const uint8_t*)&bytecode[off])

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

    // note the idx comes last - it's more convienient that way in the emitter
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

    uint32_t stringLiteral(int id, uint32_t off)
    {
        uint32_t tmp = strings[id];
        if (tmp == 0) {
            tmp = strings[id] = (uint32_t)string::fromLiteral(getstr(off));
        }
        incr(tmp);
        return tmp;
    }


    RefAction *stclo(RefAction *a, int idx, uint32_t v)
    {
    //DBG("stclo(%p, %d, %d)\n", a, idx, v);
        a->st(idx, v);
        return a;
    }

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

    RefString *mkStringSz(uint32_t sz)
    {
        RefString *res = new RefString();
        res->len = sz;
        res->data = new char[res->len + 1];
        return res;
    }

    RefString *mkStringLen(const char *lit, uint32_t len)
    {
        RefString *res = mkStringSz(len);
        memcpy(res->data, lit, len);
        res->data[len] = 0;
        //printf("MK: %s\n", res->data);
        return res;
    }

    RefString *mkString(const char *lit)
    {
        return mkStringLen(lit, strlen(lit));
    }


    namespace bitvm_number {
        void post_to_wall(int n) { printf("%d\n", n); }

        RefString *to_character(int x)
        {
            char b[2];
            b[0] = x;
            b[1] = 0;
            return mkString(b);
        }

        RefString *to_string(int x)
        {
            ManagedString m(x);
            return mkString(m.toCharArray());
        }
    }

    namespace bitvm_boolean {
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

        RefString *fromLiteral(const char *p)
        {
            RefString *res = new RefString();
            res->refcnt = 2; // never unrefed
            res->len = strlen(p);
            res->data = (char*)p;
            res->canLeak();
            return res;
        }

        RefString *mkEmpty()
        {
            if (dempty == NULL)
                dempty = fromLiteral("");

            dempty->ref();
            return dempty;
        }

        RefString *concat_op(RefString *s1, RefString *s2)
        {
            RefString *res = mkStringSz(s1->len + s2->len);
            memcpy(res->data, s1->data, s1->len);
            memcpy(res->data + s1->len, s2->data, s2->len);
            res->data[res->len] = 0;
            //printf("CONCAT "); res->print();
            return res;
        }

        RefString *concat(RefString *s1, RefString *s2)
        {
            return concat_op(s1, s2);
        }

        RefString *substring(RefString *s, int start, int length)
        {
            // If the parameters are illegal, just return a reference to the empty string.
            if (start >= s->len)
                return mkEmpty();

            // Compute a safe copy length;
            length = min(s->len-start, length);

            // Build a ManagedString from this.
            return mkStringLen(s->data+start, length);
        }

        int equals(RefString *s1, RefString *s2)
        {
            return s1->len == s2->len && memcmp(s1->data, s2->data, s1->len) == 0;
        }

        int count(RefString *s)
        {
          return s->len;
        }

        int code_at(RefString *s, int index) {
          return s->charAt(index);
        }

        RefString *at(RefString *s, int i)
        {
            char b[2];
            b[0] = code_at(s, i);
            b[1] = 0;
            return mkString(b);
        }

        int to_character_code(RefString *s) {
            return code_at(s, 0);
        }

        int to_number(RefString *s) {
          return atoi(s->data);
        }

        void post_to_wall(RefString *s) { printf("%s\n", s->data); }
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
      return new RefImage(MicroBitImage(w, h, getbytes(bitmap)));
    }

    RefImage *createImageFromString(RefString *s) {
      MicroBitImage i(s->data);
      return new RefImage(i);
    }

    RefImage *displayScreenShot()
    {
      return new RefImage(uBit.display.screenShot());
    }

    void clearImage(RefImage *i) {
      i->v.clear();
    }

    int getImagePixel(RefImage *i, int x, int y) {
      return i->v.getPixelValue(x, y);
    }

    void setImagePixel(RefImage *i, int x, int y, int value) {
      i->v.setPixelValue(x, y, value);
    }

    int getImageWidth(RefImage *i) {
      return i->v.getWidth();
    }

    // -------------------------------------------------------------------------
    // Various "show"-style functions to display and scroll things on the screen
    // -------------------------------------------------------------------------

    void showLetter(RefString *s) {
      uBit.display.print(s->charAt(0));
    }

    void scrollString(RefString *s, int delay) {
      int l = s->len;
      if (l == 0) {
        uBit.display.clear();
        uBit.sleep(delay * 5);
      } else if (l > 1) {
        ManagedString tmp(s->data, s->len);
        uBit.display.scroll(tmp, delay);
      } else {
        uBit.display.print(s->charAt(0), delay * 5);
      }
    }

    void showImage(RefImage *i, int offset) {
      uBit.display.print(i->v, -offset, 0, 0);
    }

    void scrollImage(RefImage *i, int offset, int delay) {
      if (i->v.getWidth() <= 5)
        showImage(i, 0);
      else
        uBit.display.animate(i->v, delay, offset, 0);
    }

    void plotImage(RefImage *i, int offset) {
      uBit.display.print(i->v, -offset, 0, 0, 0);
    }

    // These have their arguments rewritten by the C++ compiler.
    void showLeds(int w, int h, uint32_t bitmap) {
      RefImage *img = createImage(w,h,bitmap);
      showImage(img, 0);
      img->unref();
    }

    void showAnimation(int w, int h, uint32_t bitmap, int ms) {
      uBit.display.animate(MicroBitImage(w, h, getbytes(bitmap)), ms, 5, 0);
    }

    // -------------------------------------------------------------------------
    // BLE Events
    // -------------------------------------------------------------------------

#if false
    void generate_event(int id, int event) {
      MicroBitEvent e(id, event);
    }

    void on_event(int id, void (*a)(int)) {
      if (a != NULL) {
        uBit.MessageBus.ignore(
          id,
          MICROBIT_EVT_ANY,
          (void (*)(MicroBitEvent, void*)) callback1);
        uBit.MessageBus.listen(
          id,
          MICROBIT_EVT_ANY,
          (void (*)(MicroBitEvent, void*)) callback1,
          (void*) a);
      }
    }

    namespace events {
      void remote_control(int event) {
        micro_bit::generate_event(MES_REMOTE_CONTROL_ID,event);
      }
      void camera(int event) {
        micro_bit::generate_event(MES_CAMERA_ID, event);
      }
      void audio_recorder(int event) {
        micro_bit::generate_event(MES_AUDIO_RECORDER_ID, event);
      }
      void alert(int event) {
        micro_bit::generate_event(MES_ALERTS_ID, event);
      }
    }
#endif

    // -------------------------------------------------------------------------
    // Additions - cannot access objects
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
      ManagedString tmp(s->data, s->len);
      uBit.serial.sendString(tmp);
    }

    RefString *serialReadString()
    {
      ManagedString s = uBit.serial.readString();
      return mkStringLen(s.toCharArray(), s.length());
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

  // TODO call touch_develop::main() at the beginning of generated code
  
void error(ERROR code, int subcode)
{
    printf("Error: %d [%d]\n", code, subcode);
    die();
}


uint32_t *globals;
uint32_t *strings;
int numGlobals, numStrings;

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

int exec_binary()
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
    if (numGlobals > 30000) {
        // never executed
        linkStuff();
    }
    pc += 3; // reserved
    uint32_t startptr = (uint32_t)&bytecode[pc];
    startptr |= 1; // Thumb state
    startptr = ((uint32_t (*)())startptr)();
    printf("stop main\n");
    return startptr;
}
  
} 

#define mbit(x) (void*)bitvm_micro_bit::x,
#define mbitc(x) (void*)micro_bit::x,


namespace bitvm {

    void * const functions[] __attribute__((aligned(0x10))) = {
        (void*)0x684e35a0,
        (void*)0x7ebbb194,

        // PROC0
        mbitc(clearScreen)
        mbit(compassCalibrateEnd)
        mbit(compassCalibrateStart)
        mbit(reset)
        mbit(serialSendDisplayState)
        mbit(serialReadDisplayState)

        // PROC1
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

        // PROC2
        (void*)contract::assert,
        (void*)collection::add,
        (void*)collection::remove_at,
        (void*)refcollection::add,
        (void*)refcollection::remove_at,
        (void*)bitvm::stglb,
        (void*)bitvm::stglbRef,
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

        // PROC3
        (void*)collection::set_at,
        (void*)refcollection::set_at,
        (void*)bitvm::stfld,
        (void*)bitvm::stfldRef,
        mbit(showLeds)
        mbitc(i2c_write2)
        mbit(onButtonPressedExt)
        mbit(scrollImage)
        
        // PROC4
        mbit(showAnimation)
        mbit(setImagePixel)

        // FUNC0
        (void*)string::mkEmpty,
        (void*)collection::mk,
        (void*)refcollection::mk,
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

        // FUNC1
        (void*)boolean::not_,
        (void*)math::random,
        (void*)math::abs,
        (void*)math::sqrt,
        (void*)math::sign,
        (void*)string::count,
        (void*)string::to_character_code,
        (void*)string::to_number,
        (void*)bitvm::mkString,
        (void*)bitvm_number::to_character,
        (void*)bitvm_number::to_string,
        (void*)collection::count,
        (void*)refcollection::count,
        (void*)bitvm_boolean::to_string,
        (void*)bitvm::ldglb,
        (void*)bitvm::ldglbRef,
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

        // FUNC2
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

        // FUNC3
        (void*)math::clamp,
        (void*)string::substring,
        (void*)collection::index_of,
        (void*)refcollection::index_of,
        (void*)action::mk,
        (void*)bitvm::stclo,
        mbit(createImage)
        mbit(getImagePixel)
    };

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

// vim: ts=4 sw=4
