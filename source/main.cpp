#include "BitVM.h"

void app_main() {
  bitvm::exec_binary();
#ifdef DEBUG_MEMLEAKS
  bitvm::debugMemLeaks();
#endif
}
