#include "I2CCommon.h"

namespace touch_develop {
namespace i2c {
  I2CSimple::I2CSimple(char addr, char mask): addr(addr), mask(mask) {}

  uint8_t I2CSimple::read8(char reg){
    reg |= mask;
    char cmd2[] = { reg };
    uBit.i2c.write(addr << 1, cmd2, 1);
    char buf[1];
    uBit.i2c.read(addr << 1, buf, 1);
    return buf[0];
  }

  uint16_t I2CSimple::read16(char reg){
    reg |= mask;
    char cmd2[] = { reg };
    uBit.i2c.write(addr << 1, cmd2, 1);
    char buf[2];
    uBit.i2c.read(addr << 1, buf, 2);

    return (((uint16_t) buf[0]) << 8) + ((uint8_t) buf[1]);
  }

  int16_t I2CSimple::readS16(char reg){
    int16_t i = read16(reg);
    return (int16_t)i;
  }

  void I2CSimple::write8(char reg, char value) {
    reg |= mask;
    char c[] = { reg, value };
    uBit.i2c.write(addr << 1, c, 2);
  }
}
}
