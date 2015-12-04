/*
 * Sample BITVM extension.
 *
 * The contents of this file goes in "glue.cpp" string resource in a Touch Develop library.
 *
 * This file is actually compiled, so make sure there are *no* static initlizers and it is thus not linked.
 *
 */

// You can use any includes you want. This one comes from the microbit-touchdevelop module.
#include "I2CCommon.h"


// TODO we're still working on possiblity of including additional yotta modules.

// Provide exactly one namespace declaration. Put all your classes/functions in there.
// Make sure your namespace name is unique, in case the user is using more than one extension.
namespace coolwidget {
    // The functions in this extension do not necesserily make much sense. They
    // are only for illustrative purposes.

    
    
    
    ////////////////////////////////////////////////////////////////////////////////
    //
    // Basic usage
    // 
    ////////////////////////////////////////////////////////////////////////////////

    // The macro GLUE marks a function to be exposed to the Touch Develop library.
    // Here we write a function to compute circle area, as Touch Develop on micro:bit doesn't do floats.
    // The Touch Develop type 'Number' maps to 'int' in C++.
    GLUE int circleArea(int r)
    {
        return (int)(3.1415 * r * r);
    }

    /* 
     * In Touch Develop you would have something like this:
     *
     * atomic function circle area(radius:Number) returns Number
     * {
     *    // Compute area of a circle.
     *    // {shim:coolwidget::circleArea}
     *    var area := 3 * radius * radius
     *    return area
     * }
     *
     * The names of functions and paramters do not have to match (in fact TD
     * encourages usage of spaces in names). The order and types of parameters
     * has to match.
     *
     * The first comment is just help to be exposed in TD IDE.
     *
     * The second line binds the TD function to C++ function.
     *
     * Any function with {shim:...} annotation has its body ignored by the
     * BITVM compiler.  The body is used by the simulator though, so it's
     * always good to provide some dummy values for the simulator.
     *
     * The function is marked as 'atomic' in Touch Develop, as it does not call
     * sleep() or any related functions. Otherwise, it shouldn't be marked as atomic.
     */




    ////////////////////////////////////////////////////////////////////////////////
    //
    // Enums
    // 
    ////////////////////////////////////////////////////////////////////////////////

    #define COOLWIDGET_BUTTON_RED       1      // Decimal,
    #define COOLWIDGET_BUTTON_YELLOW    0x02   // hexadecimal,
    #define COOLWIDGET_FLAG_3           0b11   // binary,
    #define COOLWIDGET_FLAG_7           007    /* and octal are all supported. */
    #define COOLWIDGET_FLAG_12          (0x0c) // Definitions can be followed by comments, and put in parens.

    GLUE void setButtonState(int buttonId, bool state)
    {
        switch (buttonId) {
        case COOLWIDGET_BUTTON_RED:
            // ...
            break;
        case COOLWIDGET_BUTTON_YELLOW:
            // ...
            break;
        }
        wait_ms(10); // wait for it to settle.
    }

    /*
     * The TD function uses 'String' as the type of enum parameter and {enum:...} definition below.
     * There are currently no first class enums in TD.
     *
     * Specify friendly names for enum values. The function will be called like `set button("red", true)`
     *
     * function set button(button:String, enabled:Boolean)
     * {
     *    // Enable or disable a button.
     *    // {shim:coolwidget::setButtonState}
     *    // {enum:button:red=COOLWIDGET_BUTTON_RED,yellow=COOLWIDGET_BUTTON_YELLOW}
     *    serial->write line("set button state: " || button || " to " || enabled)
     *    basic->pause(10);
     * }
     *
     * We call `basic->pause(10)` to ensure timing/interleaving behavior in simulator is similar.
     * It will also prevent us from putting 'atomic' modifier on the TD function.
     *
     * We also call `serial->write line()` so it ends up in Touch Develop log. It will actually
     * never hit the serial port.
     */

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

    /*
     * atomic function get pin(pin:String) returns Number
     * {
     *    // Get current value of pin in furlongs per second.
     *    // {shim:coolwidget::getPin}
     *    // {enum:pin:A=coolwidget::PIN_A,B=coolwidget::PIN_B}
     *    return 0
     * }
     *
     * Enum names are scoped within namespace.
     */

    // The following are NOT SUPPORTED:
    #define COOLWIDGET_DOESNT_WORK          (0x01|0x10) // Only literals are supported.
    typedef enum {
        BLAH_OK = 1,
        ONLY_ONE = 2, PER_LINE = 3,  // doesn't work - only one entry per line please
        BLAH_DOESNT_WORK      // doesn't work - missing initializer
    } myenum2_t;





    ////////////////////////////////////////////////////////////////////////////////
    //
    // Strings
    // 
    ////////////////////////////////////////////////////////////////////////////////

    
    // All reference types (meaning types other than Number and Boolean) are ref-counted. 
    

    // Strings are reflected as pointers to StringData.
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

    // Any incoming ref-counted objects are guaranteed to be kept alive for the
    // duration of the function execution, but not longer.  Thus, if you want
    // to store them somewhere, either call `str->incr()`, or use ManagedString
    // which will do it for you.

    static StringData *color;
    GLUE void setColor(StringData *c)
    {
        if (color) color->decr(); // decrement any previous instance
        color = c;
        color->incr();
    }

    // Use `bitvm::mkStringData()` to construct new string buffers.
    // They come with the ref-count set to 1. This is exactly what the callers expect.
    // The buffer itself is set to all-zeros. The actual size of the buffer is len+1, to ensure NUL-termination.
    GLUE StringData *encryptString(StringData *inp)
    {
        StringData *outp = bitvm::mkStringData(inp->len);
        for (int i = 0; i < inp->len; ++i)
            outp->data[i] = inp->data[i] ^ 42;
        return outp;
    }
    




    ////////////////////////////////////////////////////////////////////////////////
    //
    // Records
    // 
    ////////////////////////////////////////////////////////////////////////////////

    // Records are constructed with `record::mk(refLen, totalLen)` function.
    // If there are only primitive fields (Numbers, Booleans) in the records,
    // refLen=0 and things are simple.
    GLUE RefRecord *readData()
    {
        RefRecord *res = record::mk(0, 2);
        res->st(0, 42);
        res->st(1, 74);
        return rec;
    }

    /*
     * object MyData {
     *    x: Number
     *    y: Number
     * }
     *
     * function read data() returns MyData
     * {
     *     // {shim::coolwidget::readData}
     *     var r := MyData->create
     *     // Dummy values for simulator.
     *     r->x := 42
     *     r->y := 74
     *     return r
     * }
     *
     */

    // Records always have all the reference type fields first, followed by all the primitive fields.
    
}

// vim: sw=4 ts=4 ai
