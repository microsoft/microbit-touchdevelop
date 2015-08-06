// mbed library revision:7:17dd2cfe0ff4
#include "microbit-touchdevelop/MicroBitTouchDevelop.h"
namespace micro_bit {
  namespace globals {
    Sprite led_x_index_legend;
    Sprite led_y_index_legend;
  }
  void calibrate();
}
namespace globals {
  String v;
  Number millig = 0;
  Number x13 = 0;
  Number x14 = 0;
  Number x12 = 0;
  Number x10 = 0;
  Number x11 = 0;
  Number x15 = 0;
  Number x16 = 0;
  Number x17 = 0;
}
void app_main();
void do_stuff();
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
    Image img;
    img = micro_bit::createImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0);
    for (Number i = 0; i < 10; ++i) {
      micro_bit::scrollImage(img, 5, 400);
    }
    uBit.compass.calibrateEnd();
  }
}
namespace literals {
  const int bitmap0_w = 10;
  const int bitmap0_h = 5;
  const uint8_t bitmap0[] = { 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, };
}

void app_main() {
  micro_bit::scrollNumber(0, 150);
  micro_bit::scrollNumber(1, 100);
  micro_bit::scrollNumber(35, 100);
}
void do_stuff() {
  micro_bit::Image img;
  img = micro_bit::createImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0);
  micro_bit::scrollNumber(0, 100);
  micro_bit::scrollNumber(1, 100);
  micro_bit::scrollNumber(35, 100);
}
