#include "MicroBitTouchDevelop.h"

namespace touch_develop {

  // ---------------------------------------------------------------------------
  // Base definitions that may be referred to by the C++ compiler.
  // ---------------------------------------------------------------------------

  namespace touch_develop {
    ManagedString mk_string(char* c) {
      return ManagedString(c);
    }
  }

  // ---------------------------------------------------------------------------
  // An adapter for the API expected by the run-time.
  // ---------------------------------------------------------------------------

  DalAdapter::DalAdapter(std::function<void()> f):
    impl_([f] (MicroBitEvent) {
      f();
    })
    {}

  DalAdapter::DalAdapter(std::function<void(int)> f):
    impl_([f] (MicroBitEvent e) {
      f(e.value);
    })
    {}

  void DalAdapter::run(MicroBitEvent e) {
    this->impl_(e);
  }

  std::map<std::pair<int, int>, unique_ptr<DalAdapter>> handlersMap;

  // ---------------------------------------------------------------------------
  // Implementation of the base TouchDevelop libraries and operations
  // ---------------------------------------------------------------------------

  namespace string {
    const ManagedString empty = ManagedString(ManagedString::EmptyString);

    bool in_range(ManagedString s, int i) {
      return i >= 0 && i < s.length();
    }

    ManagedString concat(ManagedString s1, ManagedString s2) {
      return s1 + s2;
    }

    ManagedString _(ManagedString s1, ManagedString s2) {
      return concat(s1, s2);
    }

    ManagedString substring(ManagedString s, int start, int len) {
      if (!in_range(s, start) || len < start || len < 0)
        return empty;

      return s.substring(start, len);
    }

    bool equals(ManagedString s1, ManagedString s2) {
      return s1 == s2;
    }

    int count(ManagedString s) {
      return s.length();
    }

    ManagedString at(ManagedString s, int i) {
      if (!in_range(s, i))
        return empty;

      return ManagedString(s.charAt(i));
    }

    int to_character_code(ManagedString s) {
      return s.length() > 0 ? s.charAt(0) : '\0';
    }

    int code_at(ManagedString s, int i) {
      return in_range(s, i) ? s.charAt(i) : '\0';
    }

    int to_number(ManagedString s) {
      return atoi(s.toCharArray());
    }
  }

  namespace action {
    void run(Action a) {
      if (a)
        a();
    }

    bool is_invalid(Action a) {
      if (a)
        return true;
      else
        return false;
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
    bool le(int x, int y) { return x <= y; }
    bool neq(int x, int y) { return x != y; }
    bool eq(int x, int y) { return x == y; }
    bool gt(int x, int y) { return x > y; }
    bool ge(int x, int y) { return x >= y; }
    int add(int x, int y) { return x + y; }
    int subtract(int x, int y) { return x - y; }
    int divide(int x, int y) { return x / y; }
    int multiply(int x, int y) { return x * y; }
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
    ManagedString to_string(bool x) {
      return x ? ManagedString("true") : ManagedString("false");
    }
  }


  // ---------------------------------------------------------------------------
  // Implementation of the BBC micro:bit features
  // ---------------------------------------------------------------------------

  namespace micro_bit {

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

    void on_calibrate_required (MicroBitEvent) {
      const int bitmap0_w = 10;
      const int bitmap0_h = 5;
      const uint8_t bitmap0[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, };

      uBit.compass.calibrateStart();
      uBit.display.print("Turn me around!");
      MicroBitImage img = MicroBitImage(bitmap0_w, bitmap0_h, bitmap0);
      for (Number i = 0; i < 10; ++i) {
        uBit.display.scroll(img, 5, 400);
      }
      uBit.compass.calibrateEnd();
      uBit.display.print("Please restart.");
    }

    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    // The functions below use the "enum" feature of TouchDevelop, meaning that
    // the references they are passed are always valid (hence the &-passing).
    int analogReadPin(MicroBitPin& p) {
      return p.getAnalogValue();
    }

    void analogWritePin(MicroBitPin& p, int value) {
      p.setAnalogValue(value);
    }

    void setAnalogPeriodUs(MicroBitPin& p, int value) {
      p.setAnalogPeriodUs(value);
    }

    int digitalReadPin(MicroBitPin& p) {
      return p.getDigitalValue();
    }

    void digitalWritePin(MicroBitPin& p, int value) {
      p.setDigitalValue(value);
    }

    bool isPinTouched(MicroBitPin& pin) {
      return pin.isTouched();
    }

    void onPinPressed(int pin, std::function<void()> f) {
      if (f != NULL) {
        // Forces the PIN to switch to makey-makey style detection.
        switch (pin) {
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
        onButtonPressed(pin, f);
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

    void onButtonPressedExt(int button, int event, std::function<void()> f) {
      registerWithDal(button, event, f);
    }

    void onButtonPressed(int button, std::function<void()> f) {
      onButtonPressedExt(button, MICROBIT_BUTTON_EVT_CLICK, f);
    }


    // -------------------------------------------------------------------------
    // System
    // -------------------------------------------------------------------------

    void fun_helper(std::function<void()>* f) {
      (*f)();
    }

    void fun_delete_helper(std::function<void()>* f) {
      // The fiber is done, so release associated resources and free the
      // heap-allocated closure.
      delete f;
      release_fiber();
    }

    void forever_helper(std::function<void()>* f) {
      while (true) {
        (*f)();
        pause(20);
      }
    }

    void runInBackground(std::function<void()> f) {
      if (f != NULL) {
        // The API provided by the DAL only offers a low-level, C-style,
        // void*-based callback structure. Therefore, allocate the closure on
        // the heap to make sure it fits in one word.
        auto f_allocated = new std::function<void()>(f);
        create_fiber((void(*)(void*)) fun_helper, (void*) f_allocated, (void(*)(void*)) fun_delete_helper);
      }
    }

    void pause(int ms) {
      uBit.sleep(ms);
    }

    void forever(std::function<void()> f) {
      if (f != NULL) {
        auto f_allocated = new std::function<void()>(f);
        create_fiber((void(*)(void*)) forever_helper, (void*) f_allocated, (void(*)(void*)) fun_delete_helper);
      }
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

    // Closed interval.
    inline bool in_range(int x, int l, int h) {
      return l <= x && x <= h;
    }

    int getBrightness() {
      return uBit.display.getBrightness();
    }

    void setBrightness(int v) {
      if (in_range(v, 0, 155))
        uBit.display.setBrightness(v);
    }

    void clearScreen() {
      uBit.display.image.clear();
    }

    void plot(int x, int y) {
      if (in_range(x, 0, 4) && in_range(y, 0, 4))
        uBit.display.image.setPixelValue(x, y, 1);
    }

    void unPlot(int x, int y) {
      if (in_range(x, 0, 4) && in_range(y, 0, 4))
        uBit.display.image.setPixelValue(x, y, 0);
    }

    bool point(int x, int y) {
      if (in_range(x, 0, 4) && in_range(y, 0, 4))
        return uBit.display.image.getPixelValue(x, y);
      else
        return false;
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
      if (in_range(x, 0, 4) && in_range(y, 0, 4))
        return i.getPixelValue(x, y);
      else
        return 0;
    }

    void setImagePixel(MicroBitImage i, int x, int y, int value) {
      if (in_range(x, 0, 4) && in_range(y, 0, 4))
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
      if (delay < 0)
        return;

      ManagedString t(n);
      if (n < 0 || n >= 10) {
        uBit.display.scroll(t, delay);
      } else {
        uBit.display.print(t.charAt(0), delay * 5);
      }
    }

    void scrollString(ManagedString s, int delay) {
      if (delay < 0)
        return;

      int l = s.length();
      if (l == 0) {
        uBit.display.clear();
        uBit.sleep(delay * 5);
      } else if (l > 1) {
        uBit.display.scroll(s, delay);
      } else {
        uBit.display.print(s.charAt(0), delay * 5);
      }
    }

    void plotImage(MicroBitImage i, int offset) {
      uBit.display.print(i, -offset, 0, 0, 0);
    }

    void plotLeds(int w, int h, const uint8_t* bitmap) {
      plotImage(MicroBitImage(w, h, bitmap), 0);
    }

    void showImage(MicroBitImage i, int offset) {
      uBit.display.print(i, -offset, 0, 0);
    }

    // These have their arguments rewritten by the C++ compiler.
    void showLeds(int w, int h, const uint8_t* bitmap, int delay) {
      uBit.display.print(MicroBitImage(w, h, bitmap), 0, 0, 0, delay);
    }

    void scrollImage(MicroBitImage i, int offset, int delay) {
      if (i.getWidth() <= 5)
        showImage(i, 0);
      else
        uBit.display.animate(i, delay, offset, 0);
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

    void on_event(int id, std::function<void(int)> f) {
      registerWithDal(id, MICROBIT_EVT_ANY, f);
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
      if (freq <= 0 || ms < 0 || pitchPin == NULL)
        return;
      pitchPin->setAnalogValue(512);
      pitchPin->setAnalogPeriodUs(1000000/freq);
      wait_ms(ms);
      pitchPin->setAnalogValue(0);
      wait_ms(40);
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

  // -------------------------------------------------------------------------
  // Called at start-up by the generated code (currently not enabled).
  // -------------------------------------------------------------------------
  void internal_main() {
    uBit.MessageBus.listen(
      MICROBIT_ID_COMPASS,
      MICROBIT_COMPASS_EVT_CAL_REQUIRED,
      micro_bit::on_calibrate_required);
  }
}

// vim: set ts=2 sw=2 sts=2:
