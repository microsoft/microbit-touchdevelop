#include "MicroBitCustomConfig.h"

#if __cplusplus <= 199711L
  #error The glue layer no longer builds with ARMCC. Please use GCC.
#endif

#ifndef __MICROBIT_TOUCHDEVELOP_H
#define __MICROBIT_TOUCHDEVELOP_H

#include <climits>
#include <cmath>
#include <vector>
#include <memory>
#include <functional>

#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"
#include "MemberFunctionCallback.h"

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
  // An adapter for the API expected by the run-time.
  // ---------------------------------------------------------------------------

  /**
   * The DAL API for [MicroBitMessageBus::listen] takes a [T*] and a
   * [void (T::*method)(MicroBitEvent e)]. The TouchDevelop-to-C++ compiler
   * generates either:
   * - a [void*(void)] (callback that does not capture variables)
   * - a [std::function<void*(void)>*] (callback that does capture variables)
   * - a [void*(int)] (where the integer is the [value] field of the event)
   * - a [std::function<void*(void)>*] (same as above with capture)
   *
   * The purpose of this class is to provide a generic [run] method (suitable
   * for passing to [MicroBitMessageBus::listen]) that works with any of the
   * four types above.
   */
  template <typename T>
  class DalAdapter {
    private:
      T* f;

    public:
      DalAdapter(T*);
      void run(MicroBitEvent);
  };

  template <typename T>
  DalAdapter<T>::DalAdapter(T* f) {
    this->f = f;
  }

  // This one covers both T = std::function<void*()> and T = void*(void)
  template <typename T>
  inline void DalAdapter<T>::run(MicroBitEvent) {
    (*f)();
  }

  template<>
  inline void DalAdapter<void*(int)>::run(MicroBitEvent e) {
    (*f)(e.value);
  }

  template<>
  inline void DalAdapter<std::function<void*(int)>>::run(MicroBitEvent e) {
    (*f)(e.value);
  }


  // ---------------------------------------------------------------------------
  // Implementation of the base TouchDevelop types
  // ---------------------------------------------------------------------------

  typedef int Number;
  typedef bool Boolean;
  typedef ManagedString String;
  typedef void (*Action)();
  template <typename T> using Collection_of = ManagedType<vector<T>>;
  template <typename T> using Collection = ManagedType<vector<T>>;
  template <typename T> using Ref = ManagedType<T>;

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

  // Parameterized types only work if we have the C++11-style "using" typedef.
  namespace create {
    template<typename T> Collection_of<T> collection_of();

    template<typename T> Ref<T> ref_of();
  }

  namespace collection {
    template<typename T> Number count(Collection_of<T> c);

    template<typename T> void add(Collection_of<T> c, T x);

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
    // Sensors
    // -------------------------------------------------------------------------

    int compassHeading();

    int getAcceleration(int dimension);

    void on_calibrate_required (MicroBitEvent e);


    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------

    bool isButtonPressed(int button);

    template <typename T>
    inline void onButtonPressedExt(int button, int event, T* f) {
      if (f != NULL) {
        DalAdapter<T>* adapter = new DalAdapter<T>(f);
        uBit.MessageBus.listen(button, event, adapter, &DalAdapter<T>::run);
      }
    }

    template <typename T>
    inline void onButtonPressed(int button, T* f) {
      onButtonPressedExt(button, MICROBIT_BUTTON_EVT_CLICK, f);
    }


    // -------------------------------------------------------------------------
    // Pins
    // -------------------------------------------------------------------------

    int analogReadPin(MicroBitPin& p);

    void analogWritePin(MicroBitPin& p, int value);

    void setAnalogPeriodUs(MicroBitPin& p, int value);

    int digitalReadPin(MicroBitPin& p);

    void digitalWritePin(MicroBitPin& p, int value);

    bool isPinTouched(MicroBitPin& pin);

    template <typename T>
    void onPinPressed(int pin, T f) {
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

    template <typename T>
    void on_event(int id, T* f) {
      if (f != NULL) {
        DalAdapter<T>* adapter = new DalAdapter<T>(f);
        uBit.MessageBus.listen(id, MICROBIT_EVT_ANY, adapter, &DalAdapter<T>::run);
      }
    }

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

    namespace user_types {
      struct DateTime_;
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
