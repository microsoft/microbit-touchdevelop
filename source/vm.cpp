#include "BitVM.h"

#include <stdlib.h>

namespace bitvm {

void error(ERROR code, int subcode)
{
    printf("Error: %d [%d]\n", code, subcode);
    die();
}

#define pop() (*--sp)
#define top() (sp[-1])
#define push(v) { uint32_t tmp2 = v; *sp++ = tmp2; }
#define directArg() (op >> 8)
#define nextArg() (*pc++)
#define nextArg24() ((directArg()<<16) | nextArg())

#define applyRefMaskAt(n) \
    if (refmask & (1 << n)) decr(sp[n])

void applyRefMaskCore(uint32_t refmask, uint32_t *sp)
{
    applyRefMaskAt(0);
    applyRefMaskAt(1);
    applyRefMaskAt(2);
    applyRefMaskAt(3);
    if (refmask & 0xf0) {
        applyRefMaskAt(4);
        applyRefMaskAt(5);
        applyRefMaskAt(6);
        applyRefMaskAt(7);
    }
}

#define incrpush(e) { uint32_t tmp2 = e; incr(tmp2); *sp++ = tmp2; }
#define applyRefMask() applyRefMaskCore(nextArg(), sp)



uint32_t *globals;
uint32_t *strings;
int numGlobals, numStrings;

void linkStuff()
{
    uint32_t pc = bytecode[numStrings];
    pc += (int)enums[pc];
    pc += (int)functions[pc];
    if (pc < 100) linkStuff();
}

uint32_t *allocate(uint16_t sz)
{
    uint32_t *arr = new uint32_t[sz];
    memset(arr, 0, sz * 4);
    return arr;
}

int exec_binary()
{
    uint32_t pc = 0;
    uint32_t ver = bytecode[pc++];
    printf("start!\n");
    check(ver == V2BINARY, ERR_INVALID_BINARY_HEADER);
    numGlobals = bytecode[pc++];
    numStrings = bytecode[pc++];
    globals = allocate(numGlobals);
    strings = allocate(numStrings);
    if (numGlobals > 30000) {
        // never executed
        linkStuff();
    }
    pc += 3; // reserved
    uint32_t startptr = (uint32_t)&bytecode[pc];
    startptr |= 1; // Thumb state
    startptr = ((uint32_t (*)())startptr)();
    printf("stop main\n");
    return startptr;
}

} // namespace bitvm

// vim: ts=4 sw=4
