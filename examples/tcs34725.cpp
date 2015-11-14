#include "MicroBitTouchDevelop.h"
#include "TCS34725.h"

using namespace touch_develop;
using namespace micro_bit;

bool enabled = false;

void app_main() {
  forever([=] () {
    if (enabled) {
      uint16_t r, g, b, c, k;
      tcs34725::setInterrupt(false);
      uBit.sleep(500);
      tcs34725::getRawData(&r, &g, &b, &c);
      tcs34725::setInterrupt(true);

      uBit.serial.printf("%d, %d, %d, %d\r\n", r, g, b, c);
      uBit.sleep(500);
    }
  });
  onButtonPressed(MICROBIT_ID_BUTTON_A, [=] () {
    enabled = true;
    tcs34725::enable();
    tcs34725::setIntegrationTime(tcs34725::TCS34725_INTEGRATIONTIME_24MS);
    /* tcs34725::setGain(tcs34725::TCS34725_GAIN_4X); */
  });
  onButtonPressed(MICROBIT_ID_BUTTON_B, [=] () {
    enabled = false;
    tcs34725::disable();
  });
  tcs34725::begin();
  tcs34725::disable();
}

// vim: set ts=2 sw=2 sts=2
