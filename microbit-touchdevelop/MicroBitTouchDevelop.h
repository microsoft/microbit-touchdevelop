#include "MicroBitCustomConfig.h"

#ifndef __MICROBIT_TOUCHDEVELOP_H
#define __MICROBIT_TOUCHDEVELOP_H

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
#define TD_ID(x) x

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
    ManagedString mk_string(char* c);
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
    ManagedString concat(ManagedString s1, ManagedString s2);

    ManagedString _(ManagedString s1, ManagedString s2);

    ManagedString substring(ManagedString s, int i, int j);

    bool equals(ManagedString s1, ManagedString s2);

    int count(ManagedString s);

    ManagedString at(ManagedString s, int i);

    int to_character_code(ManagedString s);

    int code_at(ManagedString s, int i);

    int to_number(ManagedString s);
  }

  namespace action {
    void run(Action a);

    bool is_invalid(Action a);
  }

  namespace math {
    int max(int x, int y);
    int min(int x, int y);
    int random(int max);
    // Unspecified behavior for int_min
    int abs(int x);
    int mod (int x, int y);

    int pow(int x, int n);

    int clamp(int l, int h, int x);

    int sqrt(int x);

    int sign(int x);
  }

  namespace number {
    bool lt(int x, int y);
    bool leq(int x, int y);
    bool neq(int x, int y);
    bool eq(int x, int y);
    bool gt(int x, int y);
    bool geq(int x, int y);
    int plus(int x, int y);
    int minus(int x, int y);
    int div(int x, int y);
    int times(int x, int y);
    ManagedString to_string(int x);
    ManagedString to_character(int x);
  }

  namespace bits {

    // See http://blog.regehr.org/archives/1063
    uint32_t rotl32c (uint32_t x, uint32_t n)
   ;

    int or_uint32(int x, int y);
    int and_uint32(int x, int y);
    int xor_uint32(int x, int y);
    int shift_left_uint32(int x, int y);
    int shift_right_uint32(int x, int y);
    int rotate_right_uint32(int x, int y);
    int rotate_left_uint32(int x, int y);
  }

  namespace boolean {
    bool or_(bool x, bool y);
    bool and_(bool x, bool y);
    bool not_(bool x);
    bool equals(bool x, bool y);
    ManagedString to_string(bool x);
  }

  // ---------------------------------------------------------------------------
  // Some extra TouchDevelop libraries (Collection, Ref, ...)
  // ---------------------------------------------------------------------------

#if __cplusplus > 199711L
  // Parameterized types only work if we have the C++11-style "using" typedef.
  namespace create {
    template<typename T> Collection_of<T> collection_of();

    template<typename T> Ref<T> ref_of();
  }

  namespace collection {
    template<typename T> Number count(Collection_of<T> c);

    template<typename T> void add(Collection_of<T> c, T x);

    // First check that [c] is valid (panic if not), then proceed to check that
    // [x] is within bounds.
    template<typename T> inline bool in_range(Collection_of<T> c, int x);

    template<typename T> T at(Collection_of<T> c, int x);

    template<typename T> void remove_at(Collection_of<T> c, int x);

    template<typename T> void set_at(Collection_of<T> c, int x, T y);

    template<typename T> Number index_of(Collection_of<T> c, T x, int start);

    template<typename T> void remove(Collection_of<T> c, T x);
  }

  namespace ref {
    template<typename T> T _get(Ref<T> x);

    template<typename T> void _set(Ref<T> x, T y);
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
    void callbackF(MicroBitEvent e, std::function<void()>* f);
#endif

    void callback(MicroBitEvent e, Action a);

    void callback1(MicroBitEvent e, void (*a)(int));


    // -------------------------------------------------------------------------
    // Sensors
    // -------------------------------------------------------------------------

    int compassHeading();

    int getAcceleration(int dimension);

    void on_calibrate_required (MicroBitEvent e);

    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    int analogReadPin(MicroBitPin& p);

    void analogWritePin(MicroBitPin& p, int value);

    void setAnalogPeriodUs(MicroBitPin& p, int value);

    int digitalReadPin(MicroBitPin& p);

    void digitalWritePin(MicroBitPin& p, int value);

    bool isPinTouched(MicroBitPin& pin);

    void onPinPressed(int pin, Action a);

    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------

    bool isButtonPressed(int button);

    void onButtonPressedExt(int button, int event, Action a);

    void onButtonPressed(int button, Action a);


#if __cplusplus > 199711L
    // Experimental support for closures compiled as C++ functions. Only works
    // for closures passed to [onButtonPressed] (all other functions would have
    // to be updated, along with [in_background]). Must figure out a way to
    // limit code duplication.
    void onButtonPressed(int button, std::function<void()>* f);
#endif

    // -------------------------------------------------------------------------
    // System
    // -------------------------------------------------------------------------

    void runInBackground(Action a);

    void pause(int ms);

    void forever_stub(void (*f)());

    void forever(void (*f)());

    int getCurrentTime();

    int i2c_read(int addr);

    void i2c_write(int addr, char c);

    void i2c_write2(int addr, int c1, int c2);

    // -------------------------------------------------------------------------
    // Screen (reading/modifying the global, mutable state of the display)
    // -------------------------------------------------------------------------

    int getBrightness();

    void setBrightness(int percentage);

    void clearScreen();

    void plot(int x, int y);

    void unPlot(int x, int y);

    bool point(int x, int y);

    // -------------------------------------------------------------------------
    // Images (helpers that create/modify a MicroBitImage)
    // -------------------------------------------------------------------------

    // Argument rewritten by the C++ emitter to be what we need
    MicroBitImage createImage(int w, int h, const uint8_t* bitmap);

    MicroBitImage createImageFromString(ManagedString s);

    void clearImage(MicroBitImage i);

    int getImagePixel(MicroBitImage i, int x, int y);

    void setImagePixel(MicroBitImage i, int x, int y, int value);

    int getImageWidth(MicroBitImage i);

    // -------------------------------------------------------------------------
    // Various "show"-style functions to display and scroll things on the screen
    // -------------------------------------------------------------------------

    void showLetter(ManagedString s);

    void showDigit(int n);

    void scrollNumber(int n, int delay);

    void scrollString(ManagedString s, int delay);

    void plotImage(MicroBitImage i, int offset);

    void plotLeds(int w, int h, const uint8_t* bitmap);

    void showImage(MicroBitImage i, int offset);

    // These have their arguments rewritten by the C++ compiler.
    void showLeds(int w, int h, const uint8_t* bitmap, int delay);

    void scrollImage(MicroBitImage i, int offset, int delay);

    void showAnimation(int w, int h, const uint8_t* bitmap, int ms);

    // -------------------------------------------------------------------------
    // BLE Events
    // -------------------------------------------------------------------------

    void generate_event(int id, int event);

    void on_event(int id, void (*a)(int));

    namespace events {
      void remote_control(int event);
      void camera(int event);
      void audio_recorder(int event);
      void alert(int event);
    }

    // -------------------------------------------------------------------------
    // Music
    // -------------------------------------------------------------------------

    void enablePitch(MicroBitPin& p);

    void pitch(int freq, int ms);
  }

  // ---------------------------------------------------------------------------
  // The DS1307 real-time clock and its i2c communication protocol
  // ---------------------------------------------------------------------------

  namespace ds1307 {

    uint8_t bcd2bin(uint8_t val);

    uint8_t bin2bcd(uint8_t val);

    // The TouchDevelop type is marked as {shim:} an exactly matches this
    // definition. It's kind of unfortunate that we have to duplicate the
    // definition.
    namespace user_types {
      struct DateTime_ ;
      typedef ManagedType<DateTime_> DateTime;
    }

    void adjust(user_types::DateTime d);

    user_types::DateTime now();
  }

  // -------------------------------------------------------------------------
  // Called at start-up by the generated code (currently not enabled).
  // -------------------------------------------------------------------------
  void internal_main();
}

#endif

// vim: set ts=2 sw=2 sts=2:
