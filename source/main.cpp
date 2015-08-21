#include "MicroBitTouchDevelop.h"
namespace touch_develop {
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
    user_types::Thing dathing();
  }
  void app_main();
  void panic(user_types::point p);
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
    user_types::Thing dathing() {
      user_types::Thing r;
      return ManagedType<user_types::Thing_>(new user_types::Thing_());
      return r;
    }
  }
  void app_main() {
    Collection<Collection<scratchpadminuslibrary::user_types::Thing>> col2;
    col2 = create::collection_of<Collection<scratchpadminuslibrary::user_types::Thing>>();
    scratchpadminuslibrary::user_types::Thing r;
    r = scratchpadminuslibrary::dathing();
  }
  void panic(user_types::point p) {
    (p.operator->() != NULL ? p->x : (uBit.panic(MICROBIT_INVALID_VALUE), p->x)) = number::plus((p.operator->() != NULL ? p->x : (uBit.panic(MICROBIT_INVALID_VALUE), p->x)), 1);
  }
}

void app_main() {
  touch_develop::app_main();
}

