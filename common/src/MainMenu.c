#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "FixP.h"
#include "Enums.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CActor.h"
#include "CRenderer.h"
#include "UI.h"
#include "SoundSystem.h"

#if !defined(ANDROID) && !defined(__EMSCRIPTEN__) && !defined(AGS) && !defined(N64) && !defined(NDS)
const char *MainMenu_options[4] = {
        "Play game", "Credits", "Help", "Quit"};

const enum EGameMenuState MainMenu_nextStateNavigation[4] = {
        kPlayGame, kCredits, kHelp,
        kQuit};

const int kMainMenuOptionsCount = 4;
#else
const char *MainMenu_options[3] = {
        "Play game", "Credits", "Help"};

const int32_t MainMenu_nextStateNavigation[3] = {
        kPlayGame, kCredits, kHelp};

const int kMainMenuOptionsCount = 3;
#endif

#ifndef TILED_BITMAPS
struct Bitmap *logoBitmap;
struct Bitmap *logo2Bitmap;
#else
struct Bitmap *logoBitmap[28];
struct Bitmap *logo2Bitmap[15];
#endif

void MainMenu_initStateCallback(int32_t tag) {
    int c;

#ifndef TILED_BITMAPS
    logoBitmap = loadBitmap("title.img");
    logo2Bitmap = loadBitmap("logo.img");
#else
    for (c = 0; c < 28; ++c) {
        char buffer[32];
        sprintf(buffer, "title_tile%04d.img", c);
        logoBitmap[c] = loadBitmap(buffer);
    }

    for (c = 0; c < 15; ++c) {
        char buffer[32];
        sprintf(buffer, "logo_tile%04d.img", c);
        logo2Bitmap[c] = loadBitmap(buffer);
    }
#endif

    biggestOption = 0;

    for (c = 0; c < kMainMenuOptionsCount; ++c) {
        const char *MainMenu_option = MainMenu_options[c];

        size_t len = strlen(MainMenu_option);

        if (len > biggestOption) {
            biggestOption = len;
        }
    }
    playSound(MAIN_MENU_THEME);
}

void MainMenu_initialPaintCallback() {
}

void MainMenu_repaintCallback(void) {
    int16_t c;

    uint8_t optionsHeight = 8 * kMainMenuOptionsCount;

    fill(0, 0, (XRES_FRAMEBUFFER), (YRES_FRAMEBUFFER), getPaletteEntry(0xFF6cb1a3), FALSE);

#ifndef TILED_BITMAPS
#ifndef AGS
    drawBitmap(0, 0, logoBitmap, 0);
#endif
    drawBitmap(XRES_FRAMEBUFFER - logo2Bitmap->width, logo2Bitmap->height / 2, logo2Bitmap, 1);
#else
    for (c = 0; c < 28; ++c) {
        drawBitmap((c & 3) * 32, (c >> 2) * 32, logoBitmap[c], 1);
    }

    for (c = 0; c < 15; ++c) {
        drawBitmap(118 + (c & 7) * 32, 45 + (c >> 3) * 32, logo2Bitmap[c], 1);
    }
#endif

    drawWindow(
            (XRES_FRAMEBUFFER / 8) - (int) biggestOption - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - (optionsHeight / 8),
            biggestOption + 2, (optionsHeight / 8) + 2,
            "Episode 0");

    for (c = 0; c < kMainMenuOptionsCount; ++c) {

        int isCursor = (cursorPosition == c)
                       && (currentPresentationState == kWaitingForInput);

        if (isCursor) {
            fill((uint16_t) (XRES_FRAMEBUFFER - (biggestOption * 8)) - 8 - 24,
                 (YRES_FRAMEBUFFER - optionsHeight) + (c * 8) - (8 * 2),
                 (biggestOption * 8) + 16, 8, getPaletteEntry(0xFF000000), FALSE);
        }

        drawTextAt((XRES_FRAMEBUFFER / 8) - biggestOption + 1 - 3,
                   (((YRES_FRAMEBUFFER / 8) + 1) - kMainMenuOptionsCount) + c - 2,
                   &MainMenu_options[c][0], isCursor ? getPaletteEntry(0xFF0000FF) : getPaletteEntry(0xFF000000));
    }
}

enum EGameMenuState MainMenu_tickCallback(enum ECommand cmd, long delta) {

    switch (cmd) {
        case kCommandUp:
            playSound(MENU_SELECTION_CHANGE_SOUND);
            --cursorPosition;
            break;
        case kCommandDown:
            playSound(MENU_SELECTION_CHANGE_SOUND);
            ++cursorPosition;
            break;
        case kCommandFire1:
        case kCommandFire2:
        case kCommandFire3:
            return MainMenu_nextStateNavigation[cursorPosition];
    }

    if (cursorPosition > (kMainMenuOptionsCount - 1)) {
        cursorPosition = (kMainMenuOptionsCount - 1);
    }

    if (cursorPosition < 0) {
        cursorPosition = 0;
    }

    return kResumeCurrentState;
}

void MainMenu_unloadStateCallback(int32_t newState) {
#ifndef TILED_BITMAPS
    releaseBitmap(logoBitmap);
    releaseBitmap(logo2Bitmap);
#else
    int c;
    for (c = 0; c < 28; ++c) {
        releaseBitmap(logoBitmap[c]);
    }

    for (c = 0; c < 15; ++c) {
        releaseBitmap(logo2Bitmap[c]);
    }
#endif
}