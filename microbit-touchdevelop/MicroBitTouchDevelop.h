#include <climits>
#include <cmath>
#include <vector>

#if __cplusplus > 199711L
#include <functional>
#endif

#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"

#define TD_NOOP(...)

namespace touch_develop {

  // ---------------------------------------------------------------------------
  // Base definitions that may be referred to by the C++ compiler.
  // ---------------------------------------------------------------------------

  enum TdError {
    TD_UNINITIALIZED_OBJECT_TYPE = 40,
    TD_OUT_OF_BOUNDS,
    TD_BAD_USAGE,
  };

  namespace touch_develop {
    ManagedString mk_string(char* c) {
      return ManagedString(c);
    }
  }

  // ---------------------------------------------------------------------------
  // Implementation of the base TouchDevelop types
  // ---------------------------------------------------------------------------

  typedef int Number;
  typedef bool Boolean;
  typedef ManagedString String;
  typedef void (*Action)();
#if __cplusplus > 199711L
  template <typename T> using Collection_of = ManagedType<vector<T>>;
  template <typename T> using Collection = ManagedType<vector<T>>;
  template <typename T> using Ref = ManagedType<T>;
#endif

  // ---------------------------------------------------------------------------
  // Implementation of the base TouchDevelop libraries and operations
  // ---------------------------------------------------------------------------

  namespace string {
    ManagedString concat(ManagedString s1, ManagedString s2) {
      return s1 + s2;
    }

    ManagedString substring(ManagedString s, int i, int j) {
      return s.substring(i, j);
    }

    bool equals(ManagedString s1, ManagedString s2) {
      return s1 == s2;
    }

    int count(ManagedString s) {
      return s.length();
    }

    ManagedString at(ManagedString s, int i) {
      return ManagedString(s.charAt(i));
    }

    int to_character_code(ManagedString s) {
      return s.length() > 0 ? s.charAt(0) : '\0';
    }

    int code_at(ManagedString s, int i) {
      return i < s.length() && i >= 0 ? s.charAt(i) : '\0';
    }

    int to_number(ManagedString s) {
      return atoi(s.toCharArray());
    }
  }

  namespace action {
    void run(Action a) {
      if (a != NULL)
        a();
    }

    bool is_invalid(Action a) {
      return a == NULL;
    }
  }

  namespace math {
    int max(int x, int y) { return x < y ? y : x; }
    int min(int x, int y) { return x < y ? x : y; }
    int random(int max) {
      if (max == INT_MIN)
        return -uBit.random(INT_MAX);
      else if (max < 0)
        return -uBit.random(-max);
      else if (max == 0)
        return 0;
      else
        return uBit.random(max);
    }
    // Unspecified behavior for int_min
    int abs(int x) { return x < 0 ? -x : x; }
    int mod (int x, int y) { return x % y; }

    int pow(int x, int n) {
      if (n < 0)
      return 0;
      int r = 1;
      while (n) {
        if (n & 1)
          r *= x;
        n >>= 1;
        x *= x;
      }
      return r;
    }

    int clamp(int l, int h, int x) {
      return x < l ? l : x > h ? h : x;
    }

    int sqrt(int x) {
      return sqrt(x);
    }

    int sign(int x) {
      return x > 0 ? 1 : (x == 0 ? 0 : -1);
    }
  }

  namespace number {
    bool lt(int x, int y) { return x < y; }
    bool leq(int x, int y) { return x <= y; }
    bool neq(int x, int y) { return x != y; }
    bool eq(int x, int y) { return x == y; }
    bool gt(int x, int y) { return x > y; }
    bool geq(int x, int y) { return x >= y; }
    int plus(int x, int y) { return x + y; }
    int minus(int x, int y) { return x - y; }
    int div(int x, int y) { return x / y; }
    int times(int x, int y) { return x * y; }
    ManagedString to_string(int x) { return ManagedString(x); }
    ManagedString to_character(int x) { return ManagedString((char) x); }
  }

  namespace bits {

    // See http://blog.regehr.org/archives/1063
    uint32_t rotl32c (uint32_t x, uint32_t n)
    {
      return (x<<n) | (x>>(-n&31));
    }

    int or_uint32(int x, int y) { return (uint32_t) x | (uint32_t) y; }
    int and_uint32(int x, int y) { return (uint32_t) x & (uint32_t) y; }
    int xor_uint32(int x, int y) { return (uint32_t) x ^ (uint32_t) y; }
    int shift_left_uint32(int x, int y) { return (uint32_t) x << y; }
    int shift_right_uint32(int x, int y) { return (uint32_t) x >> y; }
    int rotate_right_uint32(int x, int y) { return rotl32c((uint32_t) x, 32-y); }
    int rotate_left_uint32(int x, int y) { return rotl32c((uint32_t) x, y); }
  }

  namespace boolean {
    bool or_(bool x, bool y) { return x || y; }
    bool and_(bool x, bool y) { return x && y; }
    bool not_(bool x) { return !x; }
    bool equals(bool x, bool y) { return x == y; }
  }

  // ---------------------------------------------------------------------------
  // Some extra TouchDevelop libraries (Collection, Ref, ...)
  // ---------------------------------------------------------------------------

#if __cplusplus > 199711L
  // Parameterized types only work if we have the C++11-style "using" typedef.
  namespace create {
    template<typename T> Collection_of<T> collection_of() {
      return ManagedType<vector<T>>(new vector<T>());
    }

    template<typename T> Ref<T> ref_of() {
      return ManagedType<T>(new T);
    }
  }

  namespace collection {
    template<typename T> Number count(Collection_of<T> c) {
      return c->size();
    }

    template<typename T> void add(Collection_of<T> c, T x) {
      if (c.get() != NULL)
        c->push_back(x);
      else
        uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE);
    }

    // First check that [c] is valid (panic if not), then proceed to check that
    // [x] is within bounds.
    template<typename T> inline bool in_range(Collection_of<T> c, int x) {
      if (c.get() != NULL)
        return (0 <= x && x < c->size());
      else
        uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE);
    }

    template<typename T> T at(Collection_of<T> c, int x) {
      if (in_range(c, x))
        return c->at(x);
      else
        uBit.panic(TD_OUT_OF_BOUNDS);
    }

    template<typename T> void remove_at(Collection_of<T> c, int x) {
      if (!in_range(c, x))
        return;

      c->erase(c->begin()+x);
    }

    template<typename T> void set_at(Collection_of<T> c, int x, T y) {
      if (!in_range(c, x))
        return;

      c->at(x) = y;
    }

    template<typename T> Number index_of(Collection_of<T> c, T x, int start) {
      if (!in_range(c, start))
        return -1;

      int index = -1;
      for (int i = start; i < c->size(); ++i)
        if (c->at(i) == x)
          index = i;
      return index;
    }

    template<typename T> void remove(Collection_of<T> c, T x) {
      remove_at(c, index_of(c, x, 0));
    }
  }

  namespace ref {
    template<typename T> T _get(Ref<T> x) {
      return *(x.get());
    }

    template<typename T> void _set(Ref<T> x, T y) {
      *(x.get()) = y;
    }
  }
#endif


  // ---------------------------------------------------------------------------
  // Implementation of the BBC micro:bit features
  // ---------------------------------------------------------------------------

  namespace micro_bit {

    namespace user_types {
      // This one is marked as {shim:} in the TouchDevelop library, so let's
      // provide a definition for it.
      typedef MicroBitImage Image;
    }

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

#if __cplusplus > 199711L
    void callbackF(MicroBitEvent e, std::function<void()>* f) {
      (*f)();
    }
#endif

    void callback(MicroBitEvent e, Action a) {
      a();
    }

    void callback1(MicroBitEvent e, void (*a)(int)) {
      a(e.value);
    }


    // -------------------------------------------------------------------------
    // Sensors
    // -------------------------------------------------------------------------

    int compassHeading() {
      return uBit.compass.heading();
    }

    int getAcceleration(int dimension) {
      if (dimension == 0)
        return uBit.accelerometer.getX();
      else if (dimension == 1)
        return uBit.accelerometer.getY();
      else
        return uBit.accelerometer.getZ();
    }

    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    int analogReadPin(MicroBitPin& p) {
      return p.getAnalogValue();
    }

    void analogWritePin(MicroBitPin& p, int value) {
      p.setAnalogValue(value);
    }

    int digitalReadPin(MicroBitPin& p) {
      return p.getDigitalValue();
    }

    void digitalWritePin(MicroBitPin& p, int value) {
      p.setDigitalValue(value);
    }

    bool isPinTouched(MicroBitPin pin) {
      return pin.isTouched();
    }

    void onPinPressed(int pin, Action a) {
      if (a != NULL) {
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

    bool isButtonPressed(int button) {
      if (button == MICROBIT_ID_BUTTON_A)
        return uBit.buttonA.isPressed();
      else if (button == MICROBIT_ID_BUTTON_B)
        return uBit.buttonB.isPressed();
      else if (button == MICROBIT_ID_BUTTON_AB)
        return uBit.buttonAB.isPressed();
      return false;
    }

    void onButtonPressedExt(int button, int event, Action a) {
      if (a != NULL) {
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


#if __cplusplus > 199711L
    // Experimental support for closures compiled as C++ functions. Only works
    // for closures passed to [onButtonPressed] (all other functions would have
    // to be updated, along with [in_background]). Must figure out a way to
    // limit code duplication.
    void onButtonPressed(int button, std::function<void()>* f) {
      uBit.MessageBus.ignore(
        button,
        MICROBIT_BUTTON_EVT_CLICK,
        (void (*)(MicroBitEvent, void*)) callbackF);
      uBit.MessageBus.listen(
        button,
        MICROBIT_BUTTON_EVT_CLICK,
        (void (*)(MicroBitEvent, void*)) callbackF,
        (void*) f);
    }
#endif

    // -------------------------------------------------------------------------
    // System
    // -------------------------------------------------------------------------

    void runInBackground(Action a) {
      if (a != NULL)
        create_fiber(a);
    }

    void pause(int ms) {
      uBit.sleep(ms);
    }

    void forever_stub(void (*f)()) {
      while (true) {
        f();
        pause(20);
      }
    }

    void forever(void (*f)()) {
      if (f != NULL)
        create_fiber((void(*)(void*))forever_stub, (void*) f);
    }

    int getCurrentTime() {
      return uBit.systemTime();
    }

    int i2c_read(int addr) {
      char c;
      uBit.i2c.read(addr << 1, &c, 1);
      return c;
    }

    void i2c_write(int addr, char c) {
      uBit.i2c.write(addr << 1, &c, 1);
    }

    void i2c_write2(int addr, int c1, int c2) {
      char c[2];
      c[0] = (char) c1;
      c[1] = (char) c2;
      uBit.i2c.write(addr << 1, c, 2);
    }

    // -------------------------------------------------------------------------
    // Screen (reading/modifying the global, mutable state of the display)
    // -------------------------------------------------------------------------

    int getBrightness() {
      return uBit.display.getBrightness();
    }

    void setBrightness(int percentage) {
      uBit.display.setBrightness(percentage);
    }

    void clearScreen() {
      uBit.display.image.clear();
    }

    void plot(int x, int y) {
      uBit.display.image.setPixelValue(x, y, 1);
    }

    void unPlot(int x, int y) {
      uBit.display.image.setPixelValue(x, y, 0);
    }

    bool point(int x, int y) {
      return uBit.display.image.getPixelValue(x, y);
    }

    // -------------------------------------------------------------------------
    // Images (helpers that create/modify a MicroBitImage)
    // -------------------------------------------------------------------------

    // Argument rewritten by the C++ emitter to be what we need
    MicroBitImage createImage(int w, int h, const uint8_t* bitmap) {
      return MicroBitImage(w, h, bitmap);
    }

    MicroBitImage createImageFromString(ManagedString s) {
      const char* raw = s.toCharArray();
      return MicroBitImage(raw);
    }

    void clearImage(MicroBitImage i) {
      i.clear();
    }

    int getImagePixel(MicroBitImage i, int x, int y) {
      return i.getPixelValue(x, y);
    }

    void setImagePixel(MicroBitImage i, int x, int y, int value) {
      i.setPixelValue(x, y, value);
    }

    int getImageWidth(MicroBitImage i) {
      return i.getWidth();
    }

    // -------------------------------------------------------------------------
    // Various "show"-style functions to display and scroll things on the screen
    // -------------------------------------------------------------------------

    void showLetter(ManagedString s) {
      uBit.display.print(s.charAt(0));
    }

    void showDigit(int n) {
      uBit.display.print('0' + (n % 10));
    }

    void scrollNumber(int n, int delay) {
      ManagedString t(n);
      if (n < 0 || n >= 10) {
        uBit.display.scroll(t, delay);
      } else {
        uBit.display.print(t.charAt(0), delay * 5);
      }
    }

    void scrollString(ManagedString s, int delay) {
      if (s.length() > 1)
        uBit.display.scroll(s, delay);
      else {
        uBit.display.print(s.charAt(0), delay * 5);
      }
    }

    void showImage(MicroBitImage i, int offset) {
      uBit.display.print(i, -offset, 0, 0);
    }

    void scrollImage(MicroBitImage i, int offset, int delay) {
      if (i.getWidth() <= 5)
        showImage(i, 0);
      else
        uBit.display.animate(i, delay, offset, 0);
    }

    // These have their arguments rewritten by the C++ compiler.
    void plotImage(int w, int h, const uint8_t* bitmap) {
      showImage(createImage(w,h,bitmap), 0);
    }

    void showAnimation(int w, int h, const uint8_t* bitmap, int ms) {
      uBit.display.animate(MicroBitImage(w, h, bitmap), ms, 5, 0);
    }

    // -------------------------------------------------------------------------
    // BLE Events
    // -------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------
    // Music
    // -------------------------------------------------------------------------

    MicroBitPin* pitchPin = NULL;

    void enablePitch(MicroBitPin& p) {
      pitchPin = &p;
    }

    void pitch(int freq, int ms) {
      if (pitchPin == NULL) {
        uBit.display.scroll("Please call enablePitch first");
        panic(TD_BAD_USAGE);
      }
      pitchPin->setAnalogValue(512);
      pitchPin->setAnalogPeriodUs(1000000/freq);
      wait_ms(ms);
      pitchPin->setAnalogValue(0);
      wait_ms(40);
    }

    void disablePitch() {
      pitchPin->setAnalogValue(0);
    }
  }

  // ---------------------------------------------------------------------------
  // The DS1307 real-time clock and its i2c communication protocol
  // ---------------------------------------------------------------------------

  namespace ds1307 {

    const int addr = 0x68;

    uint8_t bcd2bin(uint8_t val) {
      return val - 6 * (val >> 4);
    }

    uint8_t bin2bcd(uint8_t val) {
      return val + 6 * (val / 10);
    }

    // The TouchDevelop type is marked as {shim:} an exactly matches this
    // definition. It's kind of unfortunate that we have to duplicate the
    // definition.
    namespace user_types {
      struct DateTime_ {
        Number seconds;
        Number minutes;
        Number hours;
        Number day;
        Number month;
        Number year;
      };
      typedef ManagedType<DateTime_> DateTime;
    }

    void adjust(user_types::DateTime d) {
      char commands[] = {
        0,
        bin2bcd(d->seconds),
        bin2bcd(d->minutes),
        bin2bcd(d->hours),
        0,
        bin2bcd(d->day),
        bin2bcd(d->month),
        bin2bcd(d->year - 2000)
      };
      uBit.i2c.write(addr << 1, commands, 8);
    }

    user_types::DateTime now() {
      char c = 0;
      uBit.i2c.write(addr << 1, &c, 1);

      char buf[7];
      uBit.i2c.read(addr << 1, buf, 7);

      user_types::DateTime d(new user_types::DateTime_());
      d->seconds = bcd2bin(buf[0] & 0x7F);
      d->minutes = bcd2bin(buf[1]);
      d->hours = bcd2bin(buf[2]);
      d->day = bcd2bin(buf[4]);
      d->month = bcd2bin(buf[5]);
      d->year = bcd2bin(buf[6]) + 2000;
      return d;
    }
  }

}

// vim: set ts=2 sw=2 sts=2:
