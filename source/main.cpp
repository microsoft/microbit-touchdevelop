#include "MicroBitTouchDevelop.h"
namespace touch_develop {
  namespace micro_bit {
    void calibrate();
  }
  void app_main();
  void _body_Hv41NKPqzo5vwrTQ_l0();
  void _body_xPJLp2KR4Z6JwbfI_l0();
  void _body_oAdtpoI2Ew2MywHp_l0();
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
  namespace literals {
    const int bitmap0_w = 5;
    const int bitmap0_h = 5;
    const uint8_t bitmap0[] = { 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, };
  }

  void app_main() {
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_AB, _body_Hv41NKPqzo5vwrTQ_l0);
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_xPJLp2KR4Z6JwbfI_l0);
    micro_bit::scrollNumber(1, 150);
    micro_bit::clearScreen();
    micro_bit::scrollString(touch_develop::mk_string("Hello world"), 100);
    micro_bit::forever(_body_oAdtpoI2Ew2MywHp_l0);
    if (micro_bit::isButtonPressed(MICROBIT_ID_BUTTON_AB)){
      ;
    }
  }
  void _body_Hv41NKPqzo5vwrTQ_l0() {
    ;
  }
  void _body_xPJLp2KR4Z6JwbfI_l0() {
    micro_bit::showImage(micro_bit::createImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0), 0);
  }
  void _body_oAdtpoI2Ew2MywHp_l0() {
    ;
  }
}

void app_main() {
  touch_develop::app_main();
}

