CFLAGS = -DCLI_BUILD -DINCLUDE_ITEM_DESCRIPTIONS -DENDIANESS_AWARE -DTILED_BITMAPS -DSDLGL \
	-Imenu/include                                       \
	-ISoundSystem                                       \
	-Ibase3d/include                                     \
	-I../core/include \
	-I../common/include \
	 -sUSE_SDL=2
LDFLAGS =  -O3 -s USE_ZLIB=1 -s USE_LIBPNG=1 -sUSE_SDL=2 -s --preload-file ./base.pfs --use-preload-plugins -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 --shell-file ./minimal.html
CXX = em++
CC = emcc
OBJS = 		menu/src/Main.o \
       		base3d/src/es/SDL2GL2Version.o \
       		base3d/src/es/LoadBitmap.o \
       		base3d/src/Renderer.o \
       		base3d/src/es/GLESCommon.o \
       		base3d/src/es/Renderer_Rasterization.o \
       		base3d/src/es/Renderer_Tesselation.o \
       		../common/src/MainMenu.o \
       		../common/src/Crawler.o \
       		../common/src/HelpScreen.o \
       		../common/src/CreditsScreen.o \
       		../common/src/GameMenu.o \
       		../common/src/Engine.o \
       		../common/src/UI.o \
       		../common/src/Matrices.o \
       		../common/src/HackingScreen.o \
       		../common/src/VisibilityStrategy.o \
       		../common/src/Globals.o \
       		../common/src/CTile3DProperties.o \
       		../common/src/Vec.o \
       		../common/src/Mesh.o \
       		../common/src/Common.o \
       		../common/src/FixP.o \
       		../common/src/MapWithCharKey.o \
       		../common/src/PackedFileReader.o \
       		../common/src/EDirection_Utils.o \
       		../common/src/Events.o \
       		../common/src/Dungeon.o \
       		../common/src/NullMusic.o \
       		../common/src//PCMAudio.o \
       		../common/src/HackingMinigameRules.o \
       		../core/src/Derelict.o \
       		../core/src/Core.o \
       		../core/src/Parser.o


TARGET = index.html

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

all:	$(TARGET)

serve: $(TARGET)
	python3 -m http.server

dist: $(TARGET)
	zip dist.zip index.html index.data index.js index.wasm

clean:
	rm -f $(OBJS) $(TARGET) dist.zip index.html index.data index.js index.wasm
	rm *~
	rm *.js
