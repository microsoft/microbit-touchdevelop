CC = g++ -Imicrobit-touchdevelop -DDEBUG_MEMLEAKS=1 -DDESKTOP=1 -g -Wall -Wno-format
SRCCOMMON = source/vm.cpp source/lib.cpp source/bytecode.cpp

x:
	yotta build

all:
	$(CC) -o build/vm $(SRCCOMMON) desktop/main.cpp
