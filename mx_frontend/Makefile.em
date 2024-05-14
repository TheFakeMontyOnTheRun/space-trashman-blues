CC = emcc
CXX = em++

LDFLAGS =  -O3 -s USE_ZLIB=1 -s USE_LIBPNG=1 -sUSE_SDL -s --preload-file ./base.pfs --use-preload-plugins -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 --shell-file ./minimal.html

CFLAGS=-g -c -O3     \
	-I../common/include									\
	$(SDL_INCLUDE)                                       \
	-DSDLSW                                              \
	-DVGA                                                \
	-fomit-frame-pointer                                 \
	-fno-exceptions                                      \
	-ffast-math \
	-Imenu/include                                       \
	-ISoundSystem                                       \
	-Ibase3d/include                                     \
	-I../core/include -DINCLUDE_ITEM_DESCRIPTIONS -DCLI_BUILD -sUSE_SDL -DPAGE_FLIP_ANIMATION -DENDIANESS_AWARE -DXRES_FRAMEBUFFER=320 -DYRES_FRAMEBUFFER=200

MENU_TARGET=index.html

MENU_OBJ=../common/src/MainMenu.o \
	../common/src/FixP.o \
	../common/src/Vec.o \
	../common/src/Common.o \
	../common/src/PackedFileReader.o \
	../common/src/MapWithCharKey.o \
	../common/src/CTile3DProperties.o \
	../common/src/EDirection_Utils.o \
	../common/src/Globals.o \
	../common/src/VisibilityStrategy.o \
	../common/src/Events.o \
	../common/src/Dungeon.o \
	../common/src/Crawler.o \
	../common/src/Engine.o \
	../common/src/UI.o \
	../common/src/Mesh.o \
	menu/src/Main.o \
	base3d/src/SDLVersion/SDL1Renderer.o \
	base3d/src/LoadBitmap.o \
	base3d/src/Renderer.o \
	base3d/src/RendererRasterization.o \
	base3d/src/RendererTesselation.o \
	../common/src/NullMusic.o \
	../common/src/HelpScreen.o \
	../common/src/HackingScreen.o \
	../common/src/GameMenu.o \
	../common/src/CreditsScreen.o \
	../core/src/Derelict.o \
	../core/src/Core.o \
	../common/src/HackingMinigameRules.o \
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
