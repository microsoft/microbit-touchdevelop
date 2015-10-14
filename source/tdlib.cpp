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

}

#endif

// vim: set ts=2 sw=2 sts=2:
