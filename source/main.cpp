#include "MicroBitTouchDevelop.h"
namespace scratchpadminuslibrary {
  namespace user_types {
    struct Thing_;
    typedef ManagedType< Thing_> Thing;
  }
}
namespace user_types {
  struct point_;
  typedef ManagedType< point_> point;
}
namespace scratchpadminuslibrary {
  namespace user_types {
    struct Thing_ {
      String f;
      micro_bit::user_types::Image f2;
    };
  }
}
namespace user_types {
  struct point_ {
    Number x;
    Number y;
    Number z;
  };
}
namespace micro_bit {
  void calibrate();
}
namespace scratchpadminuslibrary {
  user_types::Thing thing();
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
    user_types::Image img;
    img = micro_bit::createImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0);
    for (Number i = 0; i < 10; ++i) {
      micro_bit::scrollImage(img, 5, 400);
    }
    uBit.compass.calibrateEnd();
  }
}
namespace scratchpadminuslibrary {
  user_types::Thing thing() {
    user_types::Thing r;
    return ManagedType<user_types::Thing_>(new user_types::Thing_());
    return r;
  }
}
namespace literals {
  const int bitmap0_w = 5;
  const int bitmap0_h = 6;
  const uint8_t bitmap0[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };

  const int bitmap1_w = 5;
  const int bitmap1_h = 6;
  const uint8_t bitmap1[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
}

void app_main() {
  user_types::point point;
  point = ManagedType<user_types::point_>(new user_types::point_());
  micro_bit::scrollNumber(point->x, 150);
  scratchpadminuslibrary::user_types::Thing t;
  t = scratchpadminuslibrary::thing();
  micro_bit::plotImage(literals::bitmap0_w, literals::bitmap0_h, literals::bitmap0);
  micro_bit::user_types::Image img;
  img = micro_bit::createImage(literals::bitmap1_w, literals::bitmap1_h, literals::bitmap1);

  Collection_of<Number> c;
  collection::add(c, 1);
}
