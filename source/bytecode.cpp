#include "BitVM.h"
// This file can be overridden to compile-in a specific program
namespace bitvm {
const uint16_t bytecode[32000] 
__attribute__((aligned(0x20)))
= {
// Magic header to find it in the file
0x0801, 0x0801, 0x4242, 0x4242, 0x0801, 0x0801, 0xd83e, 0x8de9, 
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
}
