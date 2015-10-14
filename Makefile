SRCCOMMON = source/bitvm.cpp source/bytecode.cpp
HEADERS = microbit-touchdevelop/BitVM.h
TRG = build/bbc-microbit-classic-gcc/source/microbit-touchdevelop-combined.hex
TD = ../TouchDevelop

all:
	yotta build
	node scripts/generateEmbedInfo.js $(TRG) $(SRCCOMMON) $(HEADERS)
	cp build/bytecode.js $(TD)/microbit/bytecode.js
	cd $(TD) && jake
