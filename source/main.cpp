#include "MicroBitTouchDevelop.h"

void app_main()
{
  while (1)
  {
	  uBit.display.scroll("ZOMG HELLO! :)");
	  uBit.sleep(1000);
  }
}
