#include "MicroBitTouchDevelop.h"
namespace touch_develop {
  namespace micro_bit {
    void calibrate();
    void plot_frame(user_types::Image this_, Number index0);
    void play_note(Number frequency, Number ms);
  }
  void main();
  void _body_();
  namespace micro_bit {
    namespace literals {
      const int bitmap0_w = 20;
      const int bitmap0_h = 5;
      const uint8_t bitmap0[] = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, };
    }

    void calibrate() {
      // Asks the user to calibrate the compass by rotating the board.
      // {help:functions/calibrate}
      // {namespace:input}
      uBit.compass.calibrateStart();
      user_types::Image img;
      img = micro_bit::createImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0);
      for (Number i = 0; i < 10; ++i) {
        micro_bit::scrollImage(img, 5, 400);
      }
      uBit.compass.calibrateEnd();
    }
    void plot_frame(user_types::Image this_, Number index0) {
      // Plots the ``index``-th frame of the image on the screen.
      // {help:functions/plot-frame}
      // {namespace:image}
      // {weight:65}
      micro_bit::showImage(this_, number::times(5, index0));
    }
    void play_note(Number frequency, Number ms) {
      // Plays a music tone through pin `P0` for the given duration.
      // {help:functions/play-note}
      // {namespace:music}
      // {weight:90}
      // {hints:frequency:440}
      // {hints:ms:1000}
      micro_bit::enablePitch(uBit.io.P0);
      micro_bit::pitch(frequency, ms);
    }
  }
  void main() {
    ManagedType<Number> x(new Number);
    *x = 1;
    auto _body_0_ = [=] () mutable -> void {
      *x = number::plus(*x, 1);
      micro_bit::scrollNumber(*x, 150);
    };
    auto _body_0 = std::function<void ()>(_body_0_);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_0);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_B, _body_);
  }
  void _body_() {
    micro_bit::scrollNumber(0, 150);
  }
}

void app_main() {
  touch_develop::main();
}


// vim: sw=2 ts=2
