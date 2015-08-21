#include "MicroBitTouchDevelop.h"
namespace touch_develop {
  namespace micro_bit {
    void calibrate();
  }
  namespace globals {
    Number x = 0;
  }  void app_main();
  void _body_EmuyaQzpHTnrmJGF_l0();
  namespace micro_bit {
    namespace literals {
      const int bitmap0_w = 10;
      const int bitmap0_h = 5;
      const uint8_t bitmap0[] = { 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, };
    }

    void calibrate() {
      // Asks the user to calibrate the compass by moving the board in a figure-eight motion.
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
  }
  void app_main() {
    Number x0 = 0;
    x0 = 1;
    auto _body__ = [=] () -> void {
      Number y = 0;
      y = 2;
      y = number::plus(number::plus(y, globals::x), x0);
    };
    auto _body_ = new std::function<void ()>(_body__);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_);
    auto _body_0_ = [=] () -> void {
      micro_bit::scrollNumber(x0, 15);
    };
    auto _body_0 = new std::function<void ()>(_body_0_);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_0);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_B, _body_EmuyaQzpHTnrmJGF_l0);
  }
  void _body_EmuyaQzpHTnrmJGF_l0() {
    Number y0 = 0;
    y0 = 2;
    y0 = number::plus(y0, 1);
  }
}

void app_main() {
  touch_develop::app_main();
}

