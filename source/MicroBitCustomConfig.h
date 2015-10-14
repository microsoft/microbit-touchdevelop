/**
  * MicroBitCustomConfig.h
  *
  * This file is automatically included by the microbit DAL compilation
  * process. Use this to define any custom configuration options needed
  * for your build of the micro:bit runtime. 
  *
  * See microbit-dal/inc/MicroBitConfig.h for a complete list of options.
  * Any options you define here will take precedence over those defined there.
  */

#ifndef MICROBIT_CUSTOM_CONFIG_H
#define MICROBIT_CUSTOM_CONFIG_H

// Define your configuration options here.
// #define MICROBIT_DBG                                1

#undef MESSAGE_BUS_LISTENER_DEFAULT_FLAGS
#define MESSAGE_BUS_LISTENER_DEFAULT_FLAGS          MESSAGE_BUS_LISTENER_QUEUE_IF_BUSY

#endif
