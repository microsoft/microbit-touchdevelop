#include "BitVM.h"

void app_main() {
  bitvm::exec_binary((uint16_t*)bitvm::functionsAndBytecode);
}
