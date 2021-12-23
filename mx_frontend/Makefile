LDFLAGS=`sdl2-config --libs` -lm

CFLAGS=-g -c `sdl2-config --cflags `  \
	-Ibase3d/include                                     \
	-ISoundSystem                                     \
	-Imenu/include                                       \
	$(SDL_INCLUDE)                                       \
	-DSDLSW                                              \
	-DVGA                                                \
	-Wall                                                \
	-fomit-frame-pointer                                 \
	-fno-exceptions                                      \
	-ffast-math	\
	-I../core/include -I../core/include  -DINCLUDE_ITEM_DESCRIPTIONS

MENU_TARGET=game

MENU_OBJ=menu/src/MainMenu.o \
	../common/src/FixP.o \
	../common/src/MapWithCharKey.o \
	../common/src/PackedFileReader.o \
	../common/src/Vec.o \
	menu/src/Crawler.o \
	menu/src/UI.o \
	menu/src/Main.o \
	base3d/src/SDLVersion/CSDL2Renderer.o \
	base3d/src/Globals.o \
	base3d/src/CTile3DProperties.o \
	base3d/src/LoadBitmap.o \
	base3d/src/CRenderer.o \
	base3d/src/VisibilityStrategy.o \
	base3d/src/Events.o \
	base3d/src/CRenderer_Rasterization.o \
	base3d/src/EDirection_Utils.o \
	base3d/src/CRenderer_Tesselation.o \
	../core/src/Derelict.o \
	../core/src/Core.o \
	../core/src/Parser.o \
	SoundSystem/NullMusic.o \
	menu/src/HelpScreen.o \
	menu/src/GameMenu.o \
	menu/src/Engine.o \
	menu/src/CreditsScreen.o \
	menu/src/HackingScreen.o \
	base3d/src/Dungeon.o


$(MENU_TARGET):	$(MENU_OBJ)
	$(CC) -o$(MENU_TARGET) $(MENU_OBJ) $(LDFLAGS)

all:   $(MENU_TARGET)


menudata: packager
	rm -f ./menu.pfs
	ls res/*.*  | xargs ./packer
	mv ./data.pfs ./menu.pfs

clean:
	rm -f menu/src/*.o
	rm -f ../core/src/*.o
	rm -f ../common/src/*.o
	rm -f base3d/src/*.o
	rm -f common/src/*.o
	rm -f SoundSystem/*.o
	rm -f base3d/src/SDLVersion/*.o
