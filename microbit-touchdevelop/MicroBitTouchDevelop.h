#include <climits>
#include <cmath>
#include <vector>

#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"

// Base TouchDevelop types
typedef int Number;
typedef bool Boolean;
typedef ManagedString String;
typedef void (*Action)();
template <typename T> using Collection_of = ManagedType<vector<T>>;
template <typename T> using Collection = ManagedType<vector<T>>;

// Types that correspond to libraries from the TouchDevelop runtime. We might
// generate code that contains references to these types, as we can't tell if a
// global is just meant for the simulator or not.
typedef void *Board, *DateTime, *Sprite_Set, *Color, *Sprite;

namespace touch_develop {
  ManagedString mk_string(char* c) {
    return ManagedString(c);
  }
}

namespace create {
  template<typename T> Collection_of<T> collection_of() {
    return ManagedType<vector<T>>();
  }
}

namespace collection {
  template<typename T> void add(Collection_of<T> c, T x) {
    c->push_back(x);
  }

  template<typename T> inline bool in_range(Collection_of<T> c, int x) {
    return (0 <= x && x < c->size());
  }

  template<typename T> T at(Collection_of<T> c, int x) {
    if (in_range(c, x))
      return c->at(x);
    else
      uBit.panic(MICROBIT_INVALID_VALUE);
  }

  template<typename T> void remove(Collection_of<T> c, int x) {
    if (in_range(c, x))
      c->erase(c->begin()+x);
  }

  template<typename T> void set_at(Collection_of<T> c, int x, T y) {
    if (in_range(c, x))
      c->at(x) = y;
  }
}

// These should be read along with the TouchDevelop library, to make sense of
// the various constants. Same order as in the TouchDevelop library.
namespace micro_bit {
  typedef MicroBitImage Image;

  int getAcceleration(int dimension) {
    if (dimension == 0)
      return uBit.accelerometer.getX();
    else if (dimension == 1)
      return uBit.accelerometer.getY();
    else
      return uBit.accelerometer.getZ();
  }

  int analogReadPin(MicroBitPin& p) {
    return p.getAnalogValue();
  }

  void analogWritePin(MicroBitPin& p, int value) {
    p.setAnalogValue(value);
  }

  int getBrightness() {
    return uBit.display.getBrightness();
  }

  bool isButtonPressed(int button) {
    if (button == MICROBIT_ID_BUTTON_A)
      return uBit.buttonA.isPressed();
    else if (button == MICROBIT_ID_BUTTON_B)
      return uBit.buttonB.isPressed();
    else if (button == MICROBIT_ID_BUTTON_AB)
      return uBit.buttonAB.isPressed();
    return false;
  }

  bool isPinTouched(MicroBitPin pin) {
    return pin.isTouched();
  }

  void clearScreen() {
    uBit.display.image.clear();
  }

  int compassHeading() {
    return uBit.compass.heading();
  }

  // Argument rewritten by the C++ emitter to be what we need
  MicroBitImage createImage(int w, int h, const uint8_t* bitmap) {
    return MicroBitImage(w, h, bitmap);
  }

  MicroBitImage createImageFromString(ManagedString s) {
    const char* raw = s.toCharArray();
    return MicroBitImage(raw);
  }

  int getCurrentTime() {
    return uBit.systemTime();
  }

  int digitalReadPin(MicroBitPin& p) {
    return p.getDigitalValue();
  }

  void digitalWritePin(MicroBitPin& p, int value) {
    p.setDigitalValue(value);
  }

  void runInBackground(Action a) {
    if (a != NULL)
      create_fiber(a);
  }

  void callback(MicroBitEvent e, Action a) {
    a();
  }

  void onButtonPressedExt(int button, int event, Action a) {
    if (a != NULL)
      uBit.MessageBus.listen(
        button,
        event,
        (void (*)(MicroBitEvent, void*)) callback,
        (void*) a);
  }

  void onButtonPressed(int button, Action a) {
    onButtonPressedExt(button, MICROBIT_BUTTON_EVT_CLICK, a);
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
      uBit.MessageBus.listen(
        pin,
        MICROBIT_BUTTON_EVT_CLICK,
        (void (*)(MicroBitEvent, void*)) callback,
        (void*) a);
    }
  }

  void pause(int ms) {
    uBit.sleep(ms);
  }

  void plot(int x, int y) {
    uBit.display.image.setPixelValue(x, y, 1);
  }

  bool point(int x, int y) {
    return uBit.display.image.getPixelValue(x, y);
  }

  void setBrightness(int percentage) {
    uBit.display.setBrightness(percentage);
  }

  void showAnimation(int w, int h, const uint8_t* bitmap, int ms) {
    uBit.display.animate(MicroBitImage(w, h, bitmap), ms, 5, 0);
  }

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

  void unPlot(int x, int y) {
    uBit.display.image.setPixelValue(x, y, 0);
  }

  void clearImage(MicroBitImage i) {
    i.clear();
  }

  int getImagePixel(MicroBitImage i, int x, int y) {
    return i.getPixelValue(x, y);
  }

  void showImage(MicroBitImage i, int offset) {
    uBit.display.print(i, -offset, 0, 0);
  }

  void plotImage(int w, int h, const uint8_t* bitmap) {
    showImage(createImage(w,h,bitmap), 0);
  }

  void scrollImage(MicroBitImage i, int offset, int delay) {
    if (i.getWidth() <= 5)
      showImage(i, 0);
    else
      uBit.display.animate(i, delay, offset, 0);
  }

  void setImagePixel(MicroBitImage i, int x, int y, int value) {
    i.setPixelValue(x, y, value);
  }

  int getImageWidth(MicroBitImage i) {
    return i.getWidth();
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

  void generate_event(int id, int event) {
    MicroBitEvent e(id, event);
  }

  void callback1(MicroBitEvent e, void (*a)(int)) {
    a(e.value);
  }

  void on_event(int id, void (*a)(int)) {
    if (a != NULL)
      uBit.MessageBus.listen(id, MICROBIT_EVT_ANY, (void (*)(MicroBitEvent, void*))callback1, (void*)a);
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

  DynamicPwm* pwm = NULL;

  void enablePitch(MicroBitPin& p) {
    pwm = DynamicPwm::allocate(p.name);
    if (pwm == NULL) {
      uBit.display.enable();
      uBit.display.print("No pwm available");
    }
  }

  void pitch(int freq, int ms) {
    pwm->setPeriodUs(1000000/freq);
    pwm->write(.5f);
    wait_ms(ms);
    pwm->write(0);
    wait_ms(40);
  }

  void disablePitch() {
    pwm->free();
  }

}


namespace ds1307 {

  const int addr = 0x68;

  uint8_t bcd2bin(uint8_t val) {
    return val - 6 * (val >> 4);
  }

  uint8_t bin2bcd(uint8_t val) {
    return val + 6 * (val / 10);
  }

  struct DateTime {
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
  };

  void adjust(DateTime& d) {
    char commands[] = {
      0,
      bin2bcd(d.seconds),
      bin2bcd(d.minutes),
      bin2bcd(d.hours),
      0,
      bin2bcd(d.day),
      bin2bcd(d.month),
      bin2bcd(d.year - 2000)
    };
    uBit.i2c.write(addr << 1, commands, 8);
  }

  DateTime now() {
    char c = 0;
    uBit.i2c.write(addr << 1, &c, 1);

    char buf[7];
    uBit.i2c.read(addr << 1, buf, 7);

    DateTime d;
    d.seconds = bcd2bin(buf[0] & 0x7F);
    d.minutes = bcd2bin(buf[1]);
    d.hours = bcd2bin(buf[2]);
    d.day = bcd2bin(buf[4]);
    d.month = bcd2bin(buf[5]);
    d.year = bcd2bin(buf[6]) + 2000;
    return d;
  }

  int minutes() {
    return now().minutes;
  }

  int hours() {
    return now().hours;
  }

}

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

/* http://blog.regehr.org/archives/1063 */
uint32_t rotl32c (uint32_t x, uint32_t n)
{
  return (x<<n) | (x>>(-n&31));
}

namespace bits {
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

// vim: set ts=2 sw=2 sts=2:
