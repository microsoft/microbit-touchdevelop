#include "MicroBitTouchDevelop.h"

/* This module provides some useful i2c wrappers for common patterns, such as
 * writing into a register and reading a one-byte or two-byte value afterwards.
 * */

#ifndef __MICROBIT_I2CCOMMON_H
#define __MICROBIT_I2CCOMMON_H

namespace touch_develop {
namespace i2c {
  class I2CSimple {
    public:
      I2CSimple(char addr, char mask = 0);
      uint8_t   read8(char reg);
      uint16_t  read16(char reg);
      int16_t   readS16(char reg);
      void      write8(char reg, char val);
    private:
      char addr;
      char mask;
  };
}
}

#endif
