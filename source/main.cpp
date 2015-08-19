#include "MicroBitTouchDevelop.h"
namespace micro_bit {
  void calibrate();
}
void app_main();
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
void app_main() {
  Collection<Number> coll2;
  coll2 = create::collection_of<Number>();
  collection::add(coll2, 1);
  collection::add(coll2, 2);
  micro_bit::scrollNumber(collection::at(coll2, 0), 150);
  micro_bit::pause(1000);
  micro_bit::scrollNumber(collection::at(coll2, 0), 150);
  micro_bit::pause(1000);
  collection::set_at(coll2, 0, 0);
  micro_bit::scrollNumber(collection::at(coll2, 0), 150);
  micro_bit::pause(1000);
  collection::remove(coll2, 0);
  micro_bit::scrollNumber(collection::at(coll2, 0), 150);
  micro_bit::pause(1000);
}
