#!/bin/sh

SRCCOMMON=source/bitvm.cpp
HEADERS="microbit-touchdevelop/BitVM.h microbit-touchdevelop/MicroBitTouchDevelop.h"
TRG=build/bbc-microbit-classic-gcc/source/microbit-touchdevelop-combined.hex
TD=../TouchDevelop

set -x
set -e
mkdir -p build
node scripts/functionTable.js $SRCCOMMON $HEADERS yotta_modules/microbit-dal/inc/*.h
yotta build
node scripts/generateEmbedInfo.js $TRG $SRCCOMMON $HEADERS
cp build/bytecode.js $TD/microbit/bytecode.js
cp build/protos.h $TD/microbit/protos.h
cd $TD && jake
