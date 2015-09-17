#define MICROBIT_DBG 1
#define MICROBIT_HEAP_DBG 1
#include "MicroBitTouchDevelop.h"
#include "BMP085.h"
namespace touch_develop {
  namespace micro_bit {
    void calibrate();
  }
  namespace adafruit_1_2___7_segment {
    namespace globals {
      Number i = 0;
      Number offset = 0;
      Boolean colon = false;
    }    void main();
    void set_blink(Number frequency);
    void set_brightness(Number brightness);
    void init();
    void set_bitmask(Number pos, Number bitmask);
    void clear_display();
    Number digit_bitmask(Number digit);
    Number char_bitmask(String char_);
    void set_digit(Number offset0, Number digit0);
    void set_char(Number offset1, String char0);
    Number ADDR();
    Number BLINK_OFF();
    Number BLINK_2HZ();
    void set_colon(Boolean p);
    void _body_();
    void _body_0();
    void _body_1();
  }
  namespace ds1307 {
    void do_stuff();
  }
  namespace piezzo_library {
    namespace globals {
      Number A5 = 0;
    }    Number A3();
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
    Number E5();
    Number F5();
    Number G5();
  }
  namespace globals {
    Boolean colon = false;
    Number mode = 0;
  }  void main();
  void _body_();
  void _body_0();
  void _body_1();
  void _body_2();
  void _body_3();
  namespace micro_bit {
    namespace literals {
      const int bitmap0_w = 20;
      const int bitmap0_h = 5;
      const uint8_t bitmap0[] = { 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, };
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
  namespace adafruit_1_2___7_segment {
    void main() {
      // A sample main function that shows how to drive the display.
      adafruit_1_2___7_segment::init();
      adafruit_1_2___7_segment::set_brightness(7);
      adafruit_1_2___7_segment::set_blink(adafruit_1_2___7_segment::BLINK_OFF());
      micro_bit::forever(_body_);
      // Some demo code
      globals::i = 0;
      globals::offset = 0;
      micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_0);
      micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_B, _body_1);
      adafruit_1_2___7_segment::clear_display();
      adafruit_1_2___7_segment::set_digit(globals::offset, globals::i);
    }
    void set_blink(Number frequency) {
      Number BLINK_CMD = 0;
      BLINK_CMD = 128;
      Number BLINK_DISPLAYON = 0;
      BLINK_DISPLAYON = 1;
      micro_bit::i2c_write(adafruit_1_2___7_segment::ADDR(), bits::or_uint32(bits::or_uint32(BLINK_CMD, BLINK_DISPLAYON), frequency));
    }
    void set_brightness(Number brightness) {
      Number CMD_BRIGHTNESS = 0;
      CMD_BRIGHTNESS = 224;
      micro_bit::i2c_write(adafruit_1_2___7_segment::ADDR(), bits::or_uint32(CMD_BRIGHTNESS, brightness));
    }
    void init() {
      Number OSCILLATOR = 0;
      OSCILLATOR = 1;
      Number SYSTEM_SETUP = 0;
      SYSTEM_SETUP = 32;
      micro_bit::i2c_write(adafruit_1_2___7_segment::ADDR(), bits::or_uint32(SYSTEM_SETUP, OSCILLATOR));
    }
    void set_bitmask(Number pos, Number bitmask) {
      // Offset 2 is for the middle colon, so skip one if needed
      Number offset2 = 0;
      offset2 = 0;
      if (number::geq(pos, 2)){
        offset2 = 1;
      }
      // Compute the bitmask
      micro_bit::i2c_write2(adafruit_1_2___7_segment::ADDR(), number::times(number::plus(offset2, pos), 2), bitmask);
    }
    void clear_display() {
      for (Number i0 = 0; i0 < 16; ++i0) {
        micro_bit::i2c_write2(adafruit_1_2___7_segment::ADDR(), i0, 0);
      }
    }
    Number digit_bitmask(Number digit) {
      Number bitmask2;
      Number bitmask0 = 0;
      bitmask0 = 0;
      if (number::eq(digit, 1)){
        bitmask0 = 6;
      }
      else if (number::eq(digit, 2)){
        bitmask0 = 91;
      }
      else if (number::eq(digit, 3)){
        bitmask0 = 79;
      }
      else if (number::eq(digit, 4)){
        bitmask0 = 102;
      }
      else if (number::eq(digit, 5)){
        bitmask0 = 109;
      }
      else if (number::eq(digit, 6)){
        bitmask0 = 125;
      }
      else if (number::eq(digit, 7)){
        bitmask0 = 7;
      }
      else if (number::eq(digit, 8)){
        bitmask0 = 127;
      }
      else if (number::eq(digit, 9)){
        bitmask0 = 111;
      }
      else if (number::eq(digit, 0)){
        bitmask0 = 63;
      }
      return bitmask0;
      return bitmask2;
    }
    Number char_bitmask(String char_) {
      Number bitmask1;
      if (string::equals(char_, touch_develop::mk_string("-"))){
        bitmask1 = 64;
      }
      else if (string::equals(char_, touch_develop::mk_string(" "))){
        bitmask1 = 0;
      }
      else if (string::equals(char_, touch_develop::mk_string("C"))){
        bitmask1 = 57;
      }
      else if (string::equals(char_, touch_develop::mk_string("°"))){
        bitmask1 = 99;
      } else {
        bitmask1 = adafruit_1_2___7_segment::digit_bitmask(number::minus(string::to_character_code(char_), string::to_character_code(touch_develop::mk_string("0"))));
      }
      ;
      return bitmask1;
    }
    void set_digit(Number offset0, Number digit0) {
      adafruit_1_2___7_segment::set_bitmask(offset0, adafruit_1_2___7_segment::digit_bitmask(digit0));
    }
    void set_char(Number offset1, String char0) {
      adafruit_1_2___7_segment::set_bitmask(offset1, adafruit_1_2___7_segment::char_bitmask(char0));
    }
    Number ADDR() {
      Number r;
      return 112;
      return r;
    }
    Number BLINK_OFF() {
      Number r0;
      return 0;
      return r0;
    }
    Number BLINK_2HZ() {
      Number r1;
      return 2;
      return r1;
    }
    void set_colon(Boolean p) {
      Number COLON_ON = 0;
      COLON_ON = 2;
      Number COLON_OFF = 0;
      COLON_OFF = 0;
      if (p){
        micro_bit::i2c_write2(adafruit_1_2___7_segment::ADDR(), 4, COLON_ON);
      } else {
        micro_bit::i2c_write2(adafruit_1_2___7_segment::ADDR(), 4, COLON_OFF);
      }
    }
    void _body_() {
      adafruit_1_2___7_segment::set_colon(globals::colon);
      globals::colon = boolean::not_(globals::colon);
      micro_bit::pause(500);
    }
    void _body_0() {
      globals::i = math::mod(number::plus(globals::i, 1), 10);
      adafruit_1_2___7_segment::set_digit(globals::offset, globals::i);
    }
    void _body_1() {
      globals::offset = math::mod(number::plus(globals::offset, 1), 4);
      adafruit_1_2___7_segment::set_digit(globals::offset, globals::i);
    }
  }
  namespace ds1307 {
    void do_stuff() {
      user_types::DateTime DateTime0;
      DateTime0 = ManagedType<user_types::DateTime_>(new user_types::DateTime_());
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
    Number E5() {
      Number r10;
      return 659;
      return r10;
    }
    Number F5() {
      Number r11;
      return 698;
      return r11;
    }
    Number G5() {
      Number r12;
      return 784;
      return r12;
    }
  }
  void main() {
    // 0 = datetime, 1 = temperature
    globals::mode = 0;
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_B, _body_);
    // Music event handler
    micro_bit::onButtonPressed(MICROBIT_ID_BUTTON_A, _body_0);
    // Initialize the 7-segment display
    adafruit_1_2___7_segment::init();
    adafruit_1_2___7_segment::set_brightness(15);
    adafruit_1_2___7_segment::set_blink(adafruit_1_2___7_segment::BLINK_OFF());
    // Blinking fiber
    micro_bit::forever(_body_1);
    // Time fiber
    micro_bit::forever(_body_2);
    // Temperature fiber
    bmp085::begin(bmp085::BMP085_MODE_STANDARD);
    micro_bit::forever(_body_3);
  }
  void _body_() {
    globals::mode = math::mod(number::plus(globals::mode, 1), 2);
  }
  void _body_0() {
    micro_bit::enablePitch(uBit.io.P0);
    piezzo_library::vive_la_France();
    micro_bit::disablePitch();
  }
  void _body_1() {
    if (number::eq(globals::mode, 0)){
      adafruit_1_2___7_segment::set_colon(globals::colon);
      globals::colon = boolean::not_(globals::colon);
      micro_bit::pause(500);
    }
  }
  void _body_2() {
    if (number::eq(globals::mode, 0)){
      ds1307::user_types::DateTime now;
      now = ds1307::now();
      adafruit_1_2___7_segment::set_digit(0, number::div((now.operator->() != NULL ? now->hours : (uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE), now->hours)), 10));
      adafruit_1_2___7_segment::set_digit(1, math::mod((now.operator->() != NULL ? now->hours : (uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE), now->hours)), 10));
      adafruit_1_2___7_segment::set_digit(2, number::div((now.operator->() != NULL ? now->minutes : (uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE), now->minutes)), 10));
      adafruit_1_2___7_segment::set_digit(3, math::mod((now.operator->() != NULL ? now->minutes : (uBit.panic(TD_UNINITIALIZED_OBJECT_TYPE), now->minutes)), 10));
      micro_bit::pause(1000);
    }
  }
  void _body_3() {
    if (number::eq(globals::mode, 1)){
      Number t = 0;
      t = bmp085::getIntTemperature();
      adafruit_1_2___7_segment::set_digit(0, number::div(t, 10));
      adafruit_1_2___7_segment::set_digit(1, math::mod(t, 10));
      adafruit_1_2___7_segment::set_colon(false);
      adafruit_1_2___7_segment::set_char(2, touch_develop::mk_string("°"));
      adafruit_1_2___7_segment::set_char(3, touch_develop::mk_string("C"));
      micro_bit::pause(1000);
    }
  }
}

void app_main() {
  touch_develop::main();
}

