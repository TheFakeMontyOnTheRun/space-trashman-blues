CC = emcc
CXX = em++

LDFLAGS =  -O3 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_SDL=2 -s --preload-file ./base.pfs --use-preload-plugins

CFLAGS=-g -c -O3 -std=c90    \
	-Ibase3d/include                                     \
	-Imenu/include                                       \
	-ISoundSystem                                       \
	$(SDL_INCLUDE)                                       \
	-DSDLSW                                              \
	-DVGA                                                \
	-Wall                                                \
	-fpermissive										\
	-fomit-frame-pointer                                 \
	-fno-exceptions                                      \
	-ffast-math

MENU_TARGET=game.html

MENU_OBJ=menu/src/MainMenu.o \
	menu/src/Interrogation.o \
	menu/src/UI.o \
	menu/src/Main.o \
	base3d/src/SDLVersion/CSDL2Renderer.o \
	base3d/src/Globals.o \
	base3d/src/LoadBitmap.o \
	base3d/src/CRenderer.o \
	base3d/src/FixP.o \
	base3d/src/CRenderer_Rasterization.o \
	base3d/src/CPackedFileReader.o \
	menu/src/HelpScreen.o \
	menu/src/CreditsScreen.o


$(MENU_TARGET):	$(MENU_OBJ)
	$(CC) -o$(MENU_TARGET) $(MENU_OBJ) $(LDFLAGS)

all:   $(MENU_TARGET)


menudata: packager
	rm -f ./menu.pfs
	ls res/*.*  | xargs ./packer
	mv ./data.pfs ./menu.pfs

clean:
	rm -f menu/src/*.o
	rm -f base3d/src/*.o
	rm -f common/src/*.o
	rm -f SoundSystem/*.o
	rm -f base3d/src/SDLVersion/*.o
