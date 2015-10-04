// this is included in lib.cpp to avoid having to write prototypes for everything
#ifdef INCLUDE_TDLIB

#include <climits>
#include <cmath>
#include <vector>

#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"

namespace bitvm {


  // ---------------------------------------------------------------------------
  // Implementation of the BBC micro:bit features
  // ---------------------------------------------------------------------------

  namespace micro_bit {

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    typedef RefAction *Action;

    void callback(MicroBitEvent e, Action a) {
      a->run();
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

    void on_calibrate_required (MicroBitEvent e) {
      const int bitmap0_w = 10;
      const int bitmap0_h = 5;
      const uint8_t bitmap0[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, };

      uBit.compass.calibrateStart();
      uBit.display.print("Turn me around!");
      MicroBitImage img = MicroBitImage(bitmap0_w, bitmap0_h, bitmap0);
      for (int i = 0; i < 10; ++i) {
        uBit.display.scroll(img, 5, 400);
      }
      uBit.compass.calibrateEnd();
      uBit.display.print("Please restart.");
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

    void setAnalogPeriodUs(MicroBitPin& p, int value) {
      p.setAnalogPeriodUs(value);
    }

    int digitalReadPin(MicroBitPin& p) {
      return p.getDigitalValue();
    }

    void digitalWritePin(MicroBitPin& p, int value) {
      p.setDigitalValue(value);
    }

    int isPinTouched(MicroBitPin& pin) {
      return pin.isTouched();
    }

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

    int isButtonPressed(int button) {
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

    void pause(int ms) {
      uBit.sleep(ms);
    }

    void forever_stub(void *a) {
      while (true) {
        ((Action)a)->run();
        pause(20);
      }
    }

    void forever(Action a) {
      if (a != NULL) {
        a->ref();
        create_fiber(forever_stub, a);
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

    int point(int x, int y) {
      return uBit.display.image.getPixelValue(x, y);
    }

    // -------------------------------------------------------------------------
    // Images (helpers that create/modify a MicroBitImage)
    // -------------------------------------------------------------------------
    
    typedef RefStruct<MicroBitImage> RefImage;

    // Argument rewritten by the C++ emitter to be what we need
    RefImage *createImage(int w, int h, const uint8_t* bitmap) {
      return new RefImage(MicroBitImage(w, h, bitmap));
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

    void showDigit(int n) {
      uBit.display.print('0' + (n % 10));
    }

    void scrollint(int n, int delay) {
      ManagedString t(n);
      if (n < 0 || n >= 10) {
        uBit.display.scroll(t, delay);
      } else {
        uBit.display.print(t.charAt(0), delay * 5);
      }
    }

    void scrollString(RefString *s, int delay) {
      int l = s->len;
      if (l == 0) {
        uBit.display.clear();
        uBit.sleep(delay * 5);
      } else if (l > 1) {
        ManagedString tmp(s->data);
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


    /* TODO:
        uBit.serial.readDisplayState
        uBit.serial.readImage
        uBit.serial.readString
        uBit.serial.sendDisplayState
        uBit.serial.sendImage
        uBit.serial.sendString
    */
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
        int seconds;
        int minutes;
        int hours;
        int day;
        int month;
        int year;
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

  // -------------------------------------------------------------------------
  // Called at start-up by the generated code
  // -------------------------------------------------------------------------
  namespace touch_develop {
    void main() {
      uBit.MessageBus.listen(
        MICROBIT_ID_COMPASS,
        MICROBIT_COMPASS_EVT_CAL_REQUIRED,
        micro_bit::on_calibrate_required);
    }
  }

}

#endif

// vim: set ts=2 sw=2 sts=2:
