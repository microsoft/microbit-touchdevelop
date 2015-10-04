#include "BitVM.h"

int main()
{
    bitvm::exec_binary();
#ifdef DEBUG_MEMLEAKS
    bitvm::debugMemLeaks();
#endif
}


// vim: ts=4 sw=4
