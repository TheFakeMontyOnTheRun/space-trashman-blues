EE_CFLAGS += -Os -fdata-sections -ffunction-sections -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include
EE_LDFLAGS += -s -Wl,--gc-sections -ldraw -ldma -lgraph -lpacket -lc  -ldraw -lgraph -lmath3d  -lpacket -ldma

EE_CFLAGS += -Ibase3d/include        \
    -ISoundSystem           \
    -Imenu/include          \
    -DCLI_BUILD \
    -I../core/include \
    -I../common/include \
    -DINCLUDE_ITEM_DESCRIPTIONS

EE_BIN = derelict128.elf

EE_OBJS= menu/src/Main.o \
         	  ../common/src/FixP.o \
         	  ../common/src/Vec.o \
         	  ../common/src/Common.o \
         	  ../common/src/PackedFileReader.o \
         	  ../common/src/MapWithCharKey.o \
         	  base3d/src/CTile3DProperties.o \
         	  base3d/src/VisibilityStrategy.o \
         	  base3d/src/Globals.o \
         	  base3d/src/Events.o \
         	  base3d/src/EDirection_Utils.o \
         	  base3d/src/Dungeon.o \
         	  menu/src/GameMenu.o \
         	  menu/src/MainMenu.o \
         	  menu/src/Engine.o \
         	  menu/src/UI.o \
         	  menu/src/HackingScreen.o \
         	  menu/src/BattleScreen.o \
         	  menu/src/HelpScreen.o \
         	  menu/src/CreditsScreen.o \
         	  menu/src/Crawler.o \
         	  ../core/src/Derelict.o \
         	  ../core/src/Core.o \
         	  ../core/src/Parser.o \
         	  SoundSystem/NullMusic.o \
         	  base3d/src/ee/LoadBitmap.o \
         	  base3d/src/ee/Renderer_Rasterization.o \
         	  base3d/src/ee/Renderer_Tesselation.o \
         	  base3d/src/ee/Renderer.o \
         	  base3d/src/ee/EmotionEngineRenderer.o

all: $(EE_BIN)

clean:
	rm -f menu/src/*.o
	rm -f ../core/src/*.o
	rm -f ../common/src/*.o
	rm -f base3d/src/*.o
	rm -f common/src/*.o
	rm -f SoundSystem/*.o
	rm -f base3d/src/ee/*.o

# Include makefiles
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
