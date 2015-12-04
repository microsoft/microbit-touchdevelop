/*
 * Sample BITVM extension. See EXTENSIONS.md for details.
 */

#include "I2CCommon.h"


namespace coolwidget {
    GLUE int circleArea(int r)
    {
        return (int)(3.1415 * r * r);
    }

    // Real enums are fine, but every entry needs an initializer.
    typedef enum {
        PIN_A = 10,
        PIN_B = 0x20,  // the B pin
    } pin_t;

    GLUE int getPin(pin_t pin)
    {
        // ...
        return 0;
    }

    GLUE int checkSum(StringData *str)
    {
        char *ptr = str->data;
        int sum = 0;
        while (*ptr) {
            sum += *ptr;
            sum *= 13;
        }
        return sum;
    }

    static StringData *color;
    GLUE void setColor(StringData *c)
    {
        if (color) color->decr(); // decrement any previous instance
        color = c;
        color->incr();
    }

    GLUE StringData *encryptString(StringData *inp)
    {
        StringData *outp = bitvm::mkStringData(inp->len);
        for (int i = 0; i < inp->len; ++i)
            outp->data[i] = inp->data[i] ^ 42;
        return outp;
    }
    
    GLUE RefRecord *readData()
    {
        RefRecord *res = record::mk(0, 2);
        res->st(0, 42);
        res->st(1, 74);
        return res;
    }
}

// vim: sw=4 ts=4 ai
