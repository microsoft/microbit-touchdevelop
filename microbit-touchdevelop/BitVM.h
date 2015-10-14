#ifndef __BITVM_H
#define __BITVM_H

// #define DEBUG_MEMLEAKS 1

#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "MicroBit.h"
#include "MicroBitImage.h"
#include "ManagedString.h"
#include "ManagedType.h"
#define printf(...) uBit.serial.printf(__VA_ARGS__)
// #define printf(...)

#include <stdint.h>

namespace bitvm {

    typedef enum {
        ERR_INVALID_FUNCTION_HEADER = 4,
        ERR_INVALID_BINARY_HEADER = 5,
        ERR_STACK_ONRETURN = 6,
        ERR_REF_DELETED = 7,
        ERR_OUT_OF_BOUNDS = 8,
        ERR_SIZE = 9,
    } ERROR;

    const int V2BINARY = 0x4203;

    extern uint32_t *globals;
    extern uint32_t *strings;
    extern int numGlobals, numStrings;


    inline void die() { uBit.panic(42); }

    void error(ERROR code, int subcode = 0);

    inline void check(int cond, ERROR code, int subcode = 0)
    {
        if (!cond) error(code, subcode);
    }

    uint32_t exec_function(const uint16_t *pc, uint32_t *args);
    int exec_binary();


    extern void * const functions[];
    extern const int enums[];

    extern const unsigned short bytecode[];
}

#include "BitVMRefTypes.h"

#endif

// vim: ts=4 sw=4
