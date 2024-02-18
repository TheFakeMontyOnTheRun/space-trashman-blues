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
#include "PackedFileReader.h"

#ifdef EMIT_QUIT_OPTION
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

void drawGraphic(const uint8_t *shapes);

void drawLine(const uint8_t i, const uint8_t i1, const uint8_t i2, const uint8_t i3, int i4);

void realPut(uint8_t x0, uint8_t y0, int colour, void *pVoid);

#else
struct Bitmap *logoBitmap[28];
struct Bitmap *logo2Bitmap[15];
#endif

const uint8_t shapes[] = {
        6, 108, 103, 83,
        40, 63,
        42, 40,
        54, 39,
        54, 65,
        42, 66,
        40, 63,
        7, 85, 0, 0,
        32, 102,
        24, 112,
        21, 139,
        59, 139,
        59, 97,
        41, 94,
        24, 92,
        8, 43, 0, 0,
        35, 115,
        42, 116,
        59, 116,
        59, 139,
        32, 139,
        35, 132,
        35, 115,
        37, 90,
        10, 170, 0, 0,
        36, 64,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        59, 106,
        42, 110,
        35, 96,
        35, 91,
        36, 64,
        7, 85, 68, 0,
        35, 91,
        38, 74,
        59, 73,
        59, 96,
        57, 99,
        42, 105,
        35, 91,
        8, 170, 0, 0,
        59, 38,
        51, 34,
        38, 42,
        35, 65,
        39, 78,
        50, 81,
        59, 74,
        59, 38,
        9, 212, 170, 0,
        36, 71,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        58, 74,
        50, 80,
        39, 77,
        36, 67,
        8, 43, 34, 0,
        54, 88,
        55, 92,
        56, 95,
        51, 104,
        46, 99,
        45, 97,
        50, 91,
        55, 85,
        8, 85, 68, 0,
        41, 78,
        41, 85,
        43, 97,
        47, 98,
        54, 92,
        57, 75,
        50, 81,
        41, 78,
        6, 147, 172, 147,
        39, 43,
        45, 40,
        58, 45,
        58, 39,
        49, 35,
        39, 43,
        6, 170, 212, 0,
        36, 64,
        39, 75,
        40, 51,
        39, 49,
        38, 43,
        36, 64,
        6, 255, 221, 85,
        50, 78,
        53, 53,
        56, 52,
        59, 47,
        59, 72,
        50, 78,
        7, 170, 212, 0,
        39, 104,
        37, 100,
        31, 101,
        26, 115,
        29, 109,
        39, 104,
        52, 93,
        0 };


void MainMenu_initStateCallback(enum EGameMenuState tag) {
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

void drawGraphic(const uint8_t *graphic) {
    const uint8_t *ptr = graphic;
    int buffer[6];

    while (*ptr) {
        uint8_t c;
        const uint8_t npoints = *ptr++;
        const uint8_t r = *ptr++;
        const uint8_t g = *ptr++;
        const uint8_t b = *ptr++;
        const uint32_t colour = getPaletteEntry( 0xFF000000 + (b << 16) + (g << 8) + r);
        const uint8_t *shape = ptr;
        int centerX = 0;
        int centerY = 0;

        for (c = 0; c < npoints; ++c) {
            centerX += shape[2 * c];
            centerY += shape[(2 * c) + 1];
        }

        centerX /= npoints;
        centerY /= npoints;

        buffer[4] = centerX;
        buffer[5] = centerY;

        for (c = 0; c < npoints - 1; ++c) {

            buffer[0] = shape[(2 * c) + 0];
            buffer[1] = shape[(2 * c) + 1];
            buffer[2] = shape[(2 * c) + 2];
            buffer[3] = shape[(2 * c) + 3];

            fillTriangle(&buffer[0], colour);
        }
        ptr += 2 * npoints;
    }
}

void MainMenu_repaintCallback(void) {
    int16_t c;

    uint8_t optionsHeight = 8 * kMainMenuOptionsCount;

    fillRect(0, 0, (XRES_FRAMEBUFFER), (YRES_FRAMEBUFFER), getPaletteEntry(0xFF6cb1a3), FALSE);
//
//#ifndef TILED_BITMAPS
//#ifndef AGS
//    drawBitmap(0, 0, logoBitmap, 0);
//#endif
//    drawBitmap(XRES_FRAMEBUFFER - logo2Bitmap->width, logo2Bitmap->height / 2, logo2Bitmap, 1);
//#else
//    for (c = 0; c < 28; ++c) {
//        drawBitmap((c & 3) * 32, (c >> 2) * 32, logoBitmap[c], 1);
//    }
//
//    for (c = 0; c < 15; ++c) {
//        drawBitmap(118 + (c & 7) * 32, 45 + (c >> 3) * 32, logo2Bitmap[c], 1);
//    }
//#endif

    drawGraphic(shapes);

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) biggestOption - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - (optionsHeight / 8),
            biggestOption + 2,
            (optionsHeight / 8) + 2,
            "Episode 0",
            MainMenu_options,
            kMainMenuOptionsCount,
            cursorPosition);
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

void MainMenu_unloadStateCallback(enum EGameMenuState newState) {
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
