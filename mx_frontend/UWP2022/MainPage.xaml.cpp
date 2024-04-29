//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#define SDLSW




#define CLI_BUILD
#define PAGE_FLIP_ANIMATION
#define INCLUDE_ITEM_DESCRIPTIONS
#define ENDIANESS_AWARE
#define EMIT_QUIT_OPTION
#define XRES_FRAMEBUFFER 320
#define YRES_FRAMEBUFFER 200


#include "Crawler.c"
#include "MainMenu.c"
#include "HelpScreen.c"
#include "CreditsScreen.c"
#include "GameMenu.c"
#include "Engine.c"
#include "LoadBitmap.c"
#include "Renderer.c"
#include "VisibilityStrategy.c"
#include "Globals.c"
#include "CTile3DProperties.c"
#include "Vec.c"
#include "Common.c"
#include "FixP.c"
#include "Mesh.c"
#include "MapWithCharKey.c"
#include "PackedFileReader.c"
#include "EDirection_Utils.c"
#include "Dungeon.c"
#include "Events.c"
#include "RendererRasterization.c"
#include "RendererTesselation.c"
#include "NullMusic.c"
#include "UI.c"
#include "HackingScreen.c"
#include "PCMAudio.c"
#include "HackingMinigameRules.c"
#include "Derelict.c"
#include "Core.c"
#include "Parser.c"

using namespace UWP2022;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;


char* textBuffer;
extern char* messageLogBuffer;
extern enum EVisibility* visMap;
extern struct Vec2i* distances;
extern uint8_t* collisionMap;
extern struct Texture* textures;

struct Texture internalTexturesMem[TOTAL_TEXTURES];

void initHW(int argc, char** argv) {
    textBuffer = (char*)allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char*)allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t*)allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i*)allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    textures = &internalTexturesMem[0];
    itemsInMap = (uint8_t*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t*), GENERAL_MEMORY, 1);
    map = (uint8_t*)allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t*), GENERAL_MEMORY, 1);

#ifndef CD32
    initFileReader("base.pfs");
#else
    initFileReader("base.pfs");
#endif
    initZMap();
    graphicsInit();
}

void shutdownHW(void) {
    graphicsShutdown();

    disposeMem(textBuffer);
    disposeMem(messageLogBuffer);
    disposeMem(collisionMap);
    disposeMem(visMap);
    disposeMem(distances);
    disposeMem(itemsInMap);
    disposeMem(map);
}


// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}


void MainPage::CreateResources(Xaml::CanvasAnimatedControl^ sender, CanvasCreateResourcesEventArgs^ args) { }

void MainPage::Update(Xaml::CanvasAnimatedControl^ sender, Xaml::CanvasAnimatedUpdateEventArgs^ args) {}

void MainPage::Draw(Xaml::CanvasAnimatedControl^ sender, Xaml::CanvasAnimatedDrawEventArgs^ args) {}

void MainPage::OnGamepadAdded(Platform::Object^ sender, Windows::Gaming::Input::Gamepad^ args) {}

void MainPage::OnKeyDown(CoreWindow^ sender, KeyEventArgs^ args) {}



uint8_t getPaletteEntry(const uint32_t origin) {
	uint8_t shade;

	if (!(origin & 0xFF000000)) {
		return TRANSPARENCY_COLOR;
	}

	shade = 0;
	shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
	shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
	shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

	return shade;
}

void graphicsInit(void) {
	int r, g, b;

	for (r = 0; r < 256; r += 16) {
		for (g = 0; g < 256; g += 8) {
			for (b = 0; b < 256; b += 8) {
				uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
				uint8_t paletteEntry = getPaletteEntry(pixel);
				palette[paletteEntry] = pixel;
			}
		}
	}

	defaultFont = loadBitmap("font.img");
	enableSmoothMovement = TRUE;
}

void handleSystemEvents(void) {
}

void graphicsShutdown(void) {
}

void flipRenderer(void) {
}

void clearRenderer(void) {}
