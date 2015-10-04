CC = g++ -Imicrobit-touchdevelop -DDEBUG_MEMLEAKS=1 -DDESKTOP=1 -g -Wall -Wno-format
SRCCOMMON = source/vm.cpp source/lib.cpp source/bytecode.cpp
HEADERS = microbit-touchdevelop/BitVM.h microbit-touchdevelop/BitVMRefTypes.h
TRG = build/bbc-microbit-classic-gcc/source/microbit-touchdevelop-combined.hex
TD = ../TouchDevelop

all:
	yotta build
	node scripts/generateEmbedInfo.js $(TRG) $(SRCCOMMON) $(HEADERS)
	cp build/bytecode.js $(TD)/microbit/bytecode.js
	cd $(TD) && jake


desktop:
	$(CC) -o build/vm $(SRCCOMMON) desktop/main.cpp
