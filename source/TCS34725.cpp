/**************************************************************************/
/*!
    @file     Adafruit_TCS34725.cpp
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)
    Driver for the TCS34725 digital color sensors.
    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!
    @section  HISTORY
    v1.0 - First release
*/
/**************************************************************************/

#include "TCS34725.h"

namespace touch_develop {
namespace tcs34725 {

  using namespace ::touch_develop::i2c;

  I2CSimple i2c(TCS34725_ADDRESS, TCS34725_COMMAND_BIT);

  // State that was previously part of the class in the original file.
  bool _tcs34725Initialised = false;
  tcs34725Gain_t _tcs34725Gain = TCS34725_GAIN_1X;
  tcs34725IntegrationTime_t _tcs34725IntegrationTime = TCS34725_INTEGRATIONTIME_2_4MS;

  float powf(const float x, const float y) {
    return (float)(pow((double)x, (double)y));
  }

  void enable() {
    i2c.write8(TCS34725_ENABLE, TCS34725_ENABLE_PON);
    wait_ms(3);
    i2c.write8(TCS34725_ENABLE, TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
  }

  void disable() {
    /* Turn the device off to save power */
    uint8_t reg = 0;
    reg = i2c.read8(TCS34725_ENABLE);
    i2c.write8(TCS34725_ENABLE, reg & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN));
  }

  void begin () {
    /* Make sure we're actually connected */
    uint8_t x = i2c.read8(TCS34725_ID);
    if (x != 0x44 && x != 0x10) {
      uBit.serial.printf("Bad peripheral id: %x\n", x);
      uBit.panic(TD_PERIPHERAL_ERROR);
    }
    _tcs34725Initialised = true;

    /* Set default integration time and gain */
    setIntegrationTime(_tcs34725IntegrationTime);
    setGain(_tcs34725Gain);

    /* Note: by default, the device is in power down mode on bootup */
    enable();
  }

  void setIntegrationTime(tcs34725IntegrationTime_t it)
  {
    if (!_tcs34725Initialised)
      begin();

    /* Update the timing register */
    i2c.write8(TCS34725_ATIME, it);

    /* Update value placeholders */
    _tcs34725IntegrationTime = it;
  }

  void setGain(tcs34725Gain_t gain)
  {
    if (!_tcs34725Initialised)
      begin();

    /* Update the timing register */
    i2c.write8(TCS34725_CONTROL, gain);

    /* Update value placeholders */
    _tcs34725Gain = gain;
  }

  /**************************************************************************/
  /*!
      @brief  Reads the raw red, green, blue and clear channel values
  */
  /**************************************************************************/
  void getRawData (uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c)
  {
    if (!_tcs34725Initialised)
      begin();

    *c = i2c.read16(TCS34725_CDATAL);
    *r = i2c.read16(TCS34725_RDATAL);
    *g = i2c.read16(TCS34725_GDATAL);
    *b = i2c.read16(TCS34725_BDATAL);

    /* Set a delay for the integration time */
    switch (_tcs34725IntegrationTime)
    {
      case TCS34725_INTEGRATIONTIME_2_4MS:
        wait_ms(3);
        break;
      case TCS34725_INTEGRATIONTIME_24MS:
        wait_ms(24);
        break;
      case TCS34725_INTEGRATIONTIME_50MS:
        wait_ms(50);
        break;
      case TCS34725_INTEGRATIONTIME_101MS:
        wait_ms(101);
        break;
      case TCS34725_INTEGRATIONTIME_154MS:
        wait_ms(154);
        break;
      case TCS34725_INTEGRATIONTIME_700MS:
        wait_ms(700);
        break;
    }
  }


  uint16_t calculateColorTemperature(uint16_t r, uint16_t g, uint16_t b)
  {
    float X, Y, Z;      /* RGB to XYZ correlation      */
    float xc, yc;       /* Chromaticity co-ordinates   */
    float n;            /* McCamy's formula            */
    float cct;

    /* 1. Map RGB values to their XYZ counterparts.    */
    /* Based on 6500K fluorescent, 3000K fluorescent   */
    /* and 60W incandescent values for a wide range.   */
    /* Note: Y = Illuminance or lux                    */
    X = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
    Y = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
    Z = (-0.68202F * r) + (0.77073F * g) + ( 0.56332F * b);

    /* 2. Calculate the chromaticity co-ordinates      */
    xc = (X) / (X + Y + Z);
    yc = (Y) / (X + Y + Z);

    /* 3. Use McCamy's formula to determine the CCT    */
    n = (xc - 0.3320F) / (0.1858F - yc);

    /* Calculate the final CCT */
    cct = (449.0F * powf(n, 3)) + (3525.0F * powf(n, 2)) + (6823.3F * n) + 5520.33F;

    /* Return the results in degrees Kelvin */
    return (uint16_t)cct;
  }


  uint16_t calculateLux(uint16_t r, uint16_t g, uint16_t b)
  {
    float illuminance;

    /* This only uses RGB ... how can we integrate clear or calculate lux */
    /* based exclusively on clear since this might be more reliable?      */
    illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

    return (uint16_t)illuminance;
  }

  void setInterrupt(bool i) {
    uint8_t r = i2c.read8(TCS34725_ENABLE);
    if (i) {
      r |= TCS34725_ENABLE_AIEN;
    } else {
      r &= ~TCS34725_ENABLE_AIEN;
    }
    i2c.write8(TCS34725_ENABLE, r);
  }

  void clearInterrupt(void) {
    char c = 0x66;
    uBit.i2c.write(TCS34725_ADDRESS << 1, &c, 1);
  }

  void setIntLimits(uint16_t low, uint16_t high) {
     i2c.write8(0x04, low & 0xFF);
     i2c.write8(0x05, low >> 8);
     i2c.write8(0x06, high & 0xFF);
     i2c.write8(0x07, high >> 8);
  }
}}
