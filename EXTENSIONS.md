# BITVM extensions

This file documents the process of creating C++ extensions for the Touch
Develop's in-browser ARM Thumb compiler 
[called BITVM](https://www.touchdevelop.com/docs/touch-develop-in-208-bits).

Disclaimer: The functions in this extension do not necessarily make much sense.
They are only for illustrative purposes.

You can check out some examples:
* [I2C FRAM](https://microbit.co.uk/jrwwor)
* [TCS 34725](https://microbit.co.uk/vfsaap)

## How extensions works

When compiling your script, Touch Develop (TD) web app will collect the C++
source code and configuration data for all extensions (note that this can be
empty) and the requested version of the runtime, and compute a SHA256 hash of
all that data.  The hash does *not include* your TD source code (or inline ARM
Thumb assembly).  The web app will then look for a hex file named after this
hash, first in its in-browser cache, if not found on the CDN in the cloud, and
if that fails it will ask the TD cloud service to compile the extension. The
results of this compilation are then stored in the CDN.

Once the web app has the hex file, it compiles the TD source code, and appends
the resulting ARM binary code at the end. The resulting hex file is then
downloaded and you can transfer it to the device.

Thus, the cloud performs one compilation per given set of extensions used by
the script (times runtime version). Thus in the vast majority of cases,
the hex file is found in the in-browser cache or the CDN.

## Extension structure

An extension consists of Touch Develop library with an embedded string
resource named `glue.cpp`.

The `glue.cpp` looks something like this:

```cpp
#include "I2CCommon.h"

namespace coolwidget {
    GLUE int circleArea(int r)
    {
        return (int)(3.1415 * r * r);
    }

    // ...
}
```

You can use any includes you want. The `I2CCommon.h` comes from the
microbit-touchdevelop module.  We're still working on support for including
additional yotta modules.

You need to provide exactly one namespace declaration and put all your
classes/functions in there.  Make sure your namespace name is unique, in case
the user is using more than one extension.

Optionally, you can also include `glue.json` string resource, which allows for
inclusion of additional yotta dependencies and modifying the compile-time
configuration of the micro:bit, in particular disabling the BLE stack.

## Testing your extension

You need to make sure your extension compiles before you put it in the
`glue.cpp` resource. This is because the error reporting from the cloud
compiler is nearly non-existent in the current setup.

* follow instructions for building this package (microbit-touchdevelop),
  including installing yotta

* replace contents of `examples/extension.cpp` with your extension

* include a test function in the extension source, for example `void test();`

* replace `source/main.cpp` with the following:

```cpp
namespace coolwidget {
  void test();
}
void app_main() {
  coolwidget::test();
}

```

* run `yotta build` as usual; fix errors; rinse; repeat

* copy `build/bbc-microbit-classic-gcc/source/microbit-touchdevelop-combined.hex` to your micro:bit drive and make sure it works

* copy&paste contents of `examples/extension.cpp` to the `glue.cpp` resource in your library

The `printf()` function is `#defined` to serial printf. You can use it for debugging.

Never put `app_main()` in `glue.cpp` resource (and thus best not put it in
`extension.cpp` file either - keep it simple and always copy the whole file)

## Basic usage

The macro GLUE marks a function to be exposed to the TD library. Here we write
a function to compute circle area, as TD on micro:bit doesn't do floats. The TD
type `Number` maps to `int` (32 bit signed integer) in C++.

```cpp
GLUE int circleArea(int r)
{
    return (int)(3.1415 * r * r);
}
```

In Touch Develop you would have something like this:

```js
atomic function circle area(radius:Number) returns Number
{
   // Compute area of a circle.
   // {shim:coolwidget::circleArea}
   var area := 3 * radius * radius
   return area
}
```

The names of functions and parameters do not have to match (in fact TD
encourages usage of spaces in names). The order and types of parameters
has to match.

The first comment is just help to be exposed in TD IDE.

The second line binds the TD function to C++ function.

Any function with {shim:...} annotation has its body ignored by the BITVM
compiler.  The body is used by the simulator though, so it's always good to
provide some dummy values for the simulator.

The function is marked as `atomic` in Touch Develop, as it does not call
`sleep()` or any related functions. Otherwise, it shouldn't be marked as
atomic.


## Enums

C++ functions often take enum parameters, either defined as real enums or `#define`. 
This can be mapped to TD's mock enums implemented using strings.

```cpp
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
```

The TD function uses `String` as the type of enum parameter and `{enum:...}`
definition below.  There are currently no first class enums in TD.


```js
function set button(button:String, enabled:Boolean)
{
   // Enable or disable a button.
   // {shim:coolwidget::setButtonState}
   // {enum:button:red=COOLWIDGET_BUTTON_RED,yellow=COOLWIDGET_BUTTON_YELLOW}
   serial->write line("set button state: " || button || " to " || enabled)
   basic->pause(10);
}
```

Specify friendly names for enum values. The function above be called like this:


```js
set button("red", true)
```

We call `basic->pause(10)` to ensure timing/interleaving behavior in simulator
is similar to the device. Any other waiting function should be also replaced
with `pause` in the simulator.  It will also prevent us from putting `atomic`
modifier on the TD function (which would give the user wrong idea of what's going on).

We also call `serial->write line()` so it ends up in Touch Develop log. It will actually
never hit the serial port.

### Real enums

Real enums are fine, but every entry needs an initializer.

```cpp
typedef enum {
    PIN_A = 10,
    PIN_B = 0x20,  // the B pin
} pin_t;

GLUE int getPin(pin_t pin)
{
    // ...
    return 0;
}
```

In TD you need to use the namespace (unlike for `#define`).

```js
atomic function get pin(pin:String) returns Number
{
   // Get current value of pin in furlongs per second.
   // {shim:coolwidget::getPin}
   // {enum:pin:A=coolwidget::PIN_A,B=coolwidget::PIN_B}
   return 0
}
```

### Un-supported syntax

The following are NOT SUPPORTED.

```cpp
#define COOLWIDGET_DOESNT_WORK          (0x01|0x10) // Only literals are supported.
typedef enum {
    BLAH_OK = 1,
    ONLY_ONE = 2, PER_LINE = 3,  // doesn't work - only one entry per line please
    BLAH_DOESNT_WORK      // doesn't work - missing initializer
} myenum2_t;
```

These will not cause an error, but will not be visible in TD.


## Strings

    
All reference types (meaning types other than Number and Boolean) are ref-counted. 


Strings are reflected as pointers to `StringData`.

```cpp
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
```

Any incoming ref-counted objects are guaranteed to be kept alive for the
duration of the function execution, but not longer.  Thus, if you want
to store them somewhere, either call `str->incr()`, or use `ManagedString`
which will do it for you.

```cpp
static StringData *color;
GLUE void setColor(StringData *c)
{
    if (color) color->decr(); // decrement any previous instance
    color = c;
    color->incr();
}
```

Or alternatively:

```cpp
static ManagedString color;
GLUE void setColor(StringData *c)
{
    color = ManagedString(c);
}
```

Use `bitvm::mkStringData()` to construct new string buffers.  They come with
the ref-count set to 1. This is exactly what the callers expect.  The buffer
itself is set to all-zeros. The actual size of the buffer is `len + 1`, to
ensure NUL-termination.

```cpp
GLUE StringData *encryptString(StringData *inp)
{
    StringData *outp = bitvm::mkStringData(inp->len);
    for (int i = 0; i < inp->len; ++i)
        outp->data[i] = inp->data[i] ^ 42;
    return outp;
}
```

## Records

Records are constructed with `record::mk(refLen, totalLen)` function.
If there are only primitive fields (Numbers, Booleans) in the records,
`refLen == 0` and things are simple.

```cpp
GLUE RefRecord *readData()
{
    RefRecord *res = record::mk(0, 2);
    res->st(0, 42);
    res->st(1, 74);
    return res;
}
```

```js
object MyData {
   x: Number
   y: Number
}

function read data() returns MyData
{
    // {shim::coolwidget::readData}
    var r := MyData->create
    // Dummy values for simulator.
    r->x := 42
    r->y := 74
    return r
}
```

Records always have all the reference type fields first, followed by all the primitive fields.

### Records with ref-fields

TODO

## Collections

TODO

## Defining custom ref-counted types

TODO

## glue.json configuration

### Additional yotta dependencies

You can define additional dependencies in `glue.json`:

```json
{
  "dependencies": {
    "mymodule": "account/GitHubRepo#TagName"
  }
}
```

Do not use a branch name instead of tag name. It will break caching.

### Compile-time configuration of micro:bit

You can specify `#defines` to be put in custom `MicroBitConfig.h`. For example,
to disable the BLE stack, do:

```json
{
    "config": {
        "MICROBIT_BLE_ENABLED": "0"
    }
}
```

vim: sw=4 ts=4 ai
