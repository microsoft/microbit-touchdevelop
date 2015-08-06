/***************************************************************************
  This is a library for the BMP085 pressure sensor
  Designed specifically to work with the Adafruit BMP085 or BMP180 Breakout 
  ----> http://www.adafruit.com/products/391
  ----> http://www.adafruit.com/products/1603
 
  These displays use I2C to communicate, 2 pins are required to interface.
  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!
  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

namespace bmp085 {
        
    const int addr = 0x77;

    enum
    {
      BMP085_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
      BMP085_REGISTER_CHIPID             = 0xD0,
      BMP085_REGISTER_VERSION            = 0xD1,
      BMP085_REGISTER_SOFTRESET          = 0xE0,
      BMP085_REGISTER_CONTROL            = 0xF4,
      BMP085_REGISTER_TEMPDATA           = 0xF6,
      BMP085_REGISTER_PRESSUREDATA       = 0xF6,
      BMP085_REGISTER_READTEMPCMD        = 0x2E,
      BMP085_REGISTER_READPRESSURECMD    = 0x34
    };
    
    typedef enum
    {
      BMP085_MODE_ULTRALOWPOWER          = 0,
      BMP085_MODE_STANDARD               = 1,
      BMP085_MODE_HIGHRES                = 2,
      BMP085_MODE_ULTRAHIGHRES           = 3
    } bmp085_mode_t;
    
    typedef struct
    {
      int16_t  ac1;
      int16_t  ac2;
      int16_t  ac3;
      uint16_t ac4;
      uint16_t ac5;
      uint16_t ac6;
      int16_t  b1;
      int16_t  b2;
      int16_t  mb;
      int16_t  mc;
      int16_t  md;
    } bmp085_calib_data;
    
    
    static uint8_t _bmp085Mode;
    static bmp085_calib_data _bmp085_coeffs;
        
    static uint8_t read8(char i1){
        char cmd2[] = { i1 };
        uBit.i2c.write(addr << 1, cmd2, 1);
        char buf[1];
        uBit.i2c.read(addr << 1, buf, 1);
        
        return buf[0];
    }
    
    static uint16_t read16(char i1){
        char cmd2[] = { i1 };
        uBit.i2c.write(addr << 1, cmd2, 1);
        char buf[2];
        uBit.i2c.read(addr << 1, buf, 2);
        
        return (((uint16_t) buf[0]) << 8) + buf[1];
    }
    
    static int readS16(char i2){
         int16_t i = read16(i2);
         return (int16_t)i;
    }


    static void readCoefficients() {
#if BMP085_USE_DATASHEET_VALS
        _bmp085_coeffs.ac1 = 408;
        _bmp085_coeffs.ac2 = -72;
        _bmp085_coeffs.ac3 = -14383;
        _bmp085_coeffs.ac4 = 32741;
        _bmp085_coeffs.ac5 = 32757;
        _bmp085_coeffs.ac6 = 23153;
        _bmp085_coeffs.b1  = 6190;
        _bmp085_coeffs.b2  = 4;
        _bmp085_coeffs.mb  = -32768;
        _bmp085_coeffs.mc  = -8711;
        _bmp085_coeffs.md  = 2868;
        _bmp085Mode        = 0;
#else
        _bmp085_coeffs.ac1 = readS16(BMP085_REGISTER_CAL_AC1);
        _bmp085_coeffs.ac2 = readS16(BMP085_REGISTER_CAL_AC2);
        _bmp085_coeffs.ac3 = readS16(BMP085_REGISTER_CAL_AC3);
        _bmp085_coeffs.ac4 = read16(BMP085_REGISTER_CAL_AC4);
        _bmp085_coeffs.ac5 = read16(BMP085_REGISTER_CAL_AC5);
        _bmp085_coeffs.ac6 = read16(BMP085_REGISTER_CAL_AC6);
        _bmp085_coeffs.b1 = readS16(BMP085_REGISTER_CAL_B1);
        _bmp085_coeffs.b2 = readS16(BMP085_REGISTER_CAL_B2);
        _bmp085_coeffs.mb = readS16(BMP085_REGISTER_CAL_MB);
        _bmp085_coeffs.mc = readS16(BMP085_REGISTER_CAL_MC);
        _bmp085_coeffs.md = readS16(BMP085_REGISTER_CAL_MD);
#endif
    }

    void setMode(bmp085_mode_t mode) {
        _bmp085Mode = mode;
    }
   
    int readRawTemperature() {
#if BMP085_USE_DATASHEET_VALS
        return 27898;
#else
        char cmd[] = { BMP085_REGISTER_CONTROL, BMP085_REGISTER_READTEMPCMD };
        uBit.i2c.write(addr << 1, cmd, 2);
        wait_ms(5);
        return read16(BMP085_REGISTER_TEMPDATA);
#endif
    }    

        
    int readRawPressure() {
#if BMP085_USE_DATASHEET_VALS
        return 23843;
#else
        uint8_t  p8;
        uint16_t p16;
        int32_t  p32;
    
        char cmd[] = { BMP085_REGISTER_CONTROL, BMP085_REGISTER_READPRESSURECMD + (_bmp085Mode << 6)};
        uBit.i2c.write(addr << 1, cmd, 2);
        
        switch(_bmp085Mode)
        {
          case BMP085_MODE_ULTRALOWPOWER:
            wait_ms(5);
            break;
          case BMP085_MODE_STANDARD:
            wait_ms(8);
            break;
          case BMP085_MODE_HIGHRES:
            wait_ms(14);
            break;
          case BMP085_MODE_ULTRAHIGHRES:
          default:
            wait_ms(26);
            break;
        }
        
        p16 = read16(BMP085_REGISTER_PRESSUREDATA);
        p32 = (uint32_t)p16 << 8;
        p8 = read8(BMP085_REGISTER_PRESSUREDATA+2);
        p32 += p8;
        p32 >>= (8 - _bmp085Mode);
        return p32;
#endif
    }
    
    int32_t computeB5(int32_t ut) {
      int32_t X1 = (ut - (int32_t)_bmp085_coeffs.ac6) * ((int32_t)_bmp085_coeffs.ac5) >> 15;
      int32_t X2 = ((int32_t)_bmp085_coeffs.mc << 11) / (X1+(int32_t)_bmp085_coeffs.md);
      return X1 + X2;
    }
    
    void begin(bmp085_mode_t mode)
    {
      if (read8(BMP085_REGISTER_CHIPID) != 0x55)
        uBit.panic(12);
      
      /* Set the mode indicator */
      _bmp085Mode = mode;
    
      /* Coefficients need to be read once */
      readCoefficients();
    }
        
    float getPressure()
    {
      int32_t  ut = 0, up = 0, compp = 0;
      int32_t  x1, x2, b5, b6, x3, b3, p;
      uint32_t b4, b7;
    
      /* Get the raw pressure and temperature values */
      ut = readRawTemperature();
      up = readRawPressure();
    
      /* Temperature compensation */
      b5 = computeB5(ut);
    
      /* Pressure compensation */
      b6 = b5 - 4000;
      x1 = (_bmp085_coeffs.b2 * ((b6 * b6) >> 12)) >> 11;
      x2 = (_bmp085_coeffs.ac2 * b6) >> 11;
      x3 = x1 + x2;
      b3 = (((((int32_t) _bmp085_coeffs.ac1) * 4 + x3) << _bmp085Mode) + 2) >> 2;
      x1 = (_bmp085_coeffs.ac3 * b6) >> 13;
      x2 = (_bmp085_coeffs.b1 * ((b6 * b6) >> 12)) >> 16;
      x3 = ((x1 + x2) + 2) >> 2;
      b4 = (_bmp085_coeffs.ac4 * (uint32_t) (x3 + 32768)) >> 15;
      b7 = ((uint32_t) (up - b3) * (50000 >> _bmp085Mode));
    
      if (b7 < 0x80000000)
      {
        p = (b7 << 1) / b4;
      }
      else
      {
        p = (b7 / b4) << 1;
      }
    
      x1 = (p >> 8) * (p >> 8);
      x1 = (x1 * 3038) >> 16;
      x2 = (-7357 * p) >> 16;
      compp = p + ((x1 + x2 + 3791) >> 4);
    
      /* Assign compensated pressure value */
      return compp;
    }
    
    float getTemperature()
    {
      int32_t UT, X1, X2, B5;     // following ds convention
      float t;
    
      UT = readRawTemperature();
    
#if BMP085_USE_DATASHEET_VALS
        // use datasheet numbers!
        UT = 27898;
        _bmp085_coeffs.ac6 = 23153;
        _bmp085_coeffs.ac5 = 32757;
        _bmp085_coeffs.mc = -8711;
        _bmp085_coeffs.md = 2868;
#endif
    
      B5 = computeB5(UT);
      t = (B5+8) >> 4;
      t /= 10;
    
      return t;
    }
    
    int getIntTemperature() {
        return ((int) getTemperature());
    }
}