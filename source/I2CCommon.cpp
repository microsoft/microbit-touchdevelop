#include "MicroBitTouchDevelop.h"

namespace touch_develop {
  uint8_t read8(char addr, char reg){
      char cmd2[] = { reg };
      uBit.i2c.write(addr << 1, cmd2, 1);
      char buf[1];
      uBit.i2c.read(addr << 1, buf, 1);

      return buf[0];
  }

  uint16_t read16(char addr, char reg){
      char cmd2[] = { reg };
      uBit.i2c.write(addr << 1, cmd2, 1);
      char buf[2];
      uBit.i2c.read(addr << 1, buf, 2);

      return (((uint16_t) buf[0]) << 8) + buf[1];
  }

  int16_t readS16(char addr, char reg){
       int16_t i = read16(addr, reg);
       return (int16_t)i;
  }
}
