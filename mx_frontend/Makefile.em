CC = emcc
CXX = em++

LDFLAGS =  -O3 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_SDL=1 -s --preload-file ./base.pfs --use-preload-plugins -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 --shell-file ./minimal.html

CFLAGS=-g -c -O3     \
	-Ibase3d/include                                     \
	-Imenu/include                                       \
	-ISoundSystem                                       \
	-I../common/include									\
	$(SDL_INCLUDE)                                       \
	-DSDLSW                                              \
	-DVGA                                                \
	-fomit-frame-pointer                                 \
	-fno-exceptions                                      \
	-ffast-math \
	-I../core/include -DINCLUDE_ITEM_DESCRIPTIONS -DCLI_BUILD

MENU_TARGET=game.html

MENU_OBJ=menu/src/MainMenu.o \
	../common/src/FixP.o \
	../common/src/Vec.o \
	../common/src/Common.o \
	../common/src/PackedFileReader.o \
	../common/src/MapWithCharKey.o \
	menu/src/Crawler.o \
	menu/src/Engine.o \
	menu/src/UI.o \
	menu/src/Main.o \
	base3d/src/Globals.o \
	base3d/src/SDLVersion/SDL1Renderer.o \
	base3d/src/CTile3DProperties.o \
	base3d/src/LoadBitmap.o \
	base3d/src/CRenderer.o \
	base3d/src/VisibilityStrategy.o \
	base3d/src/Events.o \
	base3d/src/CRenderer_Rasterization.o \
	base3d/src/EDirection_Utils.o \
	base3d/src/CRenderer_Tesselation.o \
	SoundSystem/NullMusic.o \
	menu/src/HelpScreen.o \
	menu/src/BattleScreen.o \
	menu/src/HackingScreen.o \
	menu/src/GameMenu.o \
	menu/src/CreditsScreen.o \
	base3d/src/Dungeon.o \
	../core/src/Derelict.o \
	../core/src/Core.o \
	../core/src/Parser.o


$(MENU_TARGET):	$(MENU_OBJ)
	$(CC) -o$(MENU_TARGET) $(MENU_OBJ) $(LDFLAGS)

all:   $(MENU_TARGET)

serve: $(MENU_TARGET)
	python3 -m http.server

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