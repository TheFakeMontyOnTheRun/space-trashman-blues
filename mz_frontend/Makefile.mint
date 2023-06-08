CXX = m68k-atari-mint-g++
CC = m68k-atari-mint-gcc
STRIP=m68k-atari-mint-strip
CFLAGS = -std=c99 -DATARIST -I../common/include -I../core/include -DCLI_BUILD -Imenu/include -ISoundSystem -c -m68000 -s -fexpensive-optimizations -fomit-frame-pointer -Ofast -DAGA5BPP -IAtariVersion -I../core/include -I../common/include -IEngine3D -DRES128X128 -DCAN_PICK_OBJECT_AT_ANY_DISTANCE -DSUPPORTS_ROOM_TRANSITION_ANIMATION
LDFLAGS =  -s -mcrt=nix13

MENU_TARGET=game.prg

MENU_OBJ= \
    Engine3D/demo.o \
    AtariVersion/AtariRenderer.o \
    ../core/src/Core.o \
    ../core/src/Derelict.o \
    ../common/src/PackedFileReader.o \
    ../common/src/Common.o

$(MENU_TARGET):	$(MENU_OBJ)
	$(CC) -o$(MENU_TARGET) $(MENU_OBJ) $(LDFLAGS)
	$(STRIP) $(MENU_TARGET)

all:   $(MENU_TARGET)

clean:
	rm -f Engine3D/*.o
	rm -f AtariVersion/*.o
	rm -f ../core/src/*.o
	rm -f ../common/src/*.o
