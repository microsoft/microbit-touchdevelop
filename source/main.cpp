#include "MicroBitTouchDevelop.h"
namespace micro_bit {
  void calibrate();
}
namespace piezzo_library {
  namespace globals {
    Number A5 = 0;
  }
  Number A3();
  Number B3();
  Number C4();
  Number D4();
  Number E4();
  Number F4();
  Number G4();
  Number A4();
  Number B4();
  Number C5();
  Number D5();
  void vive_la_France();
  void happyBirthday();
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
namespace piezzo_library {
  Number A3() {
    Number r;
    return 220;
    return r;
  }
  Number B3() {
    Number r0;
    return 247;
    return r0;
  }
  Number C4() {
    Number r1;
    return 262;
    return r1;
  }
  Number D4() {
    Number r2;
    return 294;
    return r2;
  }
  Number E4() {
    Number r3;
    return 330;
    return r3;
  }
  Number F4() {
    Number r4;
    return 349;
    return r4;
  }
  Number G4() {
    Number r5;
    return 392;
    return r5;
  }
  Number A4() {
    Number r6;
    return 440;
    return r6;
  }
  Number B4() {
    Number r7;
    return 494;
    return r7;
  }
  Number C5() {
    Number r8;
    return 523;
    return r8;
  }
  Number D5() {
    Number r9;
    return 587;
    return r9;
  }
  void vive_la_France() {
    Number dc = 0;
    dc = 180;
    micro_bit::pitch(piezzo_library::D4(), dc);
    micro_bit::pitch(piezzo_library::D4(), number::times(3, dc));
    micro_bit::pitch(piezzo_library::D4(), dc);
    micro_bit::pitch(piezzo_library::G4(), number::times(4, dc));
    micro_bit::pitch(piezzo_library::G4(), number::times(4, dc));
    micro_bit::pitch(piezzo_library::A4(), number::times(4, dc));
    micro_bit::pitch(piezzo_library::A4(), number::times(4, dc));
    micro_bit::pitch(piezzo_library::D5(), number::times(6, dc));
    micro_bit::pitch(piezzo_library::B4(), number::times(2, dc));
    micro_bit::pitch(piezzo_library::G4(), number::times(3, dc));
    micro_bit::pitch(piezzo_library::G4(), dc);
    micro_bit::pitch(piezzo_library::B4(), number::times(3, dc));
    micro_bit::pitch(piezzo_library::G4(), dc);
    micro_bit::pitch(piezzo_library::E4(), number::times(4, dc));
    micro_bit::pitch(piezzo_library::C5(), number::times(8, dc));
    micro_bit::pitch(piezzo_library::A4(), number::times(3, dc));
    micro_bit::pitch(piezzo_library::F4(), dc);
    micro_bit::pitch(piezzo_library::G4(), number::times(8, dc));
    micro_bit::pause(number::times(4, dc));
  }
  void happyBirthday() {
    Number dc0 = 0;
    dc0 = 180;
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::D4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::C4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::F4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::E4(), number::times(8, dc0));
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::D4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::C4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::G4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::F4(), number::times(8, dc0));
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::C4(), dc0);
    micro_bit::pitch(piezzo_library::C5(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::A4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::F4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::E4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::D4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::B4(), dc0);
    micro_bit::pitch(piezzo_library::B4(), number::times(2, dc0));
    micro_bit::pitch(piezzo_library::A4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::F4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::G4(), number::times(4, dc0));
    micro_bit::pitch(piezzo_library::F4(), number::times(5, dc0));
    micro_bit::pause(number::times(4, dc0));
  }
}
void app_main() {
  micro_bit::enablePitch(uBit.io.P0);
  piezzo_library::happyBirthday();
  micro_bit::disablePitch();
}
