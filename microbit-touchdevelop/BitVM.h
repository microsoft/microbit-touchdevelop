#ifndef __BITVM_H
#define __BITVM_H

// #define DEBUG_MEMLEAKS 1

#ifndef DESKTOP
#define MICROBIT
#endif

#ifdef MICROBIT
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"
#define printf(...) uBit.serial.printf(__VA_ARGS__)
// #define printf(...)
#endif

#include <stdint.h>

namespace bitvm {

    /*OPCODES*/ typedef enum {
        NOOP,         // S0
        RET0,         // S0
        RET1,         // S-1

        LDZERO,       // S1
        LDCONST8,     // S1
        LDCONST16,    // S1
        LDCONST32,    // S1

        LDPTR,        // S1
        LDARG,        // S1
        LDLOC,        // S1
        LDARGREF,     // S1
        LDLOCREF,     // S1
        LDFLD,        // S0
        LDFLDREF,     // S0
        LDGLB,        // S1
        LDGLBREF,     // S1
        LDSTRREF,     // S1
        LDENUM,       // S1
        STGLB,        // S-1
        STGLBREF,     // S-1
        STFLD,        // S-2
        STFLDREF,     // S-2
        STCLO,        // S-1
        STLOC,        // S-1
        STLOCREF,     // S-1
        CLRLOCREF,    // S0
        POP,          // S-1
        POPREF,       // S-1
        NOT,          // S0
        ISNULL,       // S0
        NEG,          // S0
        JMP,          // S0
        JMPZ,         // S-1
        JMPNZ,        // S-1

        UCALLPROC,    // SX
        UCALLFUNC,    // SX
        FLATUCALLPROC,    // SX
        FLATUCALLFUNC,    // SX

        FLATCALL0PROC,    // S0
        FLATCALL1PROC,    // S-1
        FLATCALL2PROC,    // S-2
        FLATCALL3PROC,    // S-3
        FLATCALL4PROC,    // S-4

        FLATCALL0FUNC,    // S1
        FLATCALL1FUNC,    // S0
        FLATCALL2FUNC,    // S-1
        FLATCALL3FUNC,    // S-2
        FLATCALL4FUNC,    // S-3

        CALL0PROC,    // S0
        CALL1PROC,    // S-1
        CALL2PROC,    // S-2
        CALL3PROC,    // S-3
        CALL4PROC,    // S-4

        CALL0FUNC,    // S1
        CALL1FUNC,    // S0
        CALL2FUNC,    // S-1
        CALL3FUNC,    // S-2
        CALL4FUNC,    // S-3
    } OPCODE;

    typedef enum {
        ERR_BAD_OPCODE = 1,
        ERR_STACK_OVERFLOW = 2,
        ERR_STACK_UNDERFLOW = 3,
        ERR_INVALID_FUNCTION_HEADER = 4,
        ERR_INVALID_BINARY_HEADER = 5,
        ERR_STACK_ONRETURN = 6,
        ERR_REF_DELETED = 7,
        ERR_OUT_OF_BOUNDS = 8,
        ERR_SIZE = 9,
    } ERROR;

    const int V1FUNC = 0x4201;
    const int V1BINARY = 0x4202;

    extern uint32_t *globals;
    extern uint32_t *strings;
    extern int numGlobals, numStrings;


    #ifdef MICROBIT
    inline void die() { uBit.panic(42); }
    #else
    inline void die() { exit(1); }
    #endif

    void error(ERROR code, int subcode = 0);

    inline void check(int cond, ERROR code, int subcode = 0)
    {
        if (!cond) error(code, subcode);
    }

    uint32_t exec_function(const uint16_t *pc, uint32_t *args);
    int exec_binary();

    inline int min(int a, int b)
    {
        return (a < b ? a : b);
    }

    inline int max(int a, int b)
    {
        return (a > b ? a : b);
    }


    extern const void *callProc0[];
    extern const void *callProc1[];
    extern const void *callProc2[];
    extern const void *callProc3[];
    extern const void *callProc4[];
    extern const void *callFunc0[];
    extern const void *callFunc1[];
    extern const void *callFunc2[];
    extern const void *callFunc3[];
    extern const void *callFunc4[];
    extern const int enums[];

    extern const unsigned short bytecode[];
}

#include "BitVMRefTypes.h"

#endif

// vim: ts=4 sw=4
