#include "MicroBitTouchDevelop.h"

void app_main()
{
  while (1)
  {
	  uBit.display.scrollString("ZOMG HELLO! :)");
	  uBit.sleep(1000);
  }
}
