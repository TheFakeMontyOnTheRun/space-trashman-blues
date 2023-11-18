#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Engine.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Mesh.h"
#include "CRenderer.h"
#include "UI.h"

const char *CreditsScreen_options[1] = {"Back"};

enum EGameMenuState CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

const int16_t CreditsScreen_optionsCount = 1;

#ifndef TILED_BITMAPS
struct Bitmap *monty;
#else
struct Bitmap *monty[4];
#endif

void CreditsScreen_initStateCallback(enum EGameMenuState tag) {

    struct StaticBuffer textFile = loadBinaryFileFromPath("Credits.txt");
    mainText = textBuffer;
    memFill(textBuffer, 0, TEXT_BUFFER_SIZE);
    memCopyToFrom(textBuffer, (void *) textFile.data, textFile.size);
    disposeDiskBuffer(textFile);

#ifndef TILED_BITMAPS
    monty = loadBitmap("monty.img");
#else
    monty[0] = loadBitmap("monty_tile0000.img");
    monty[1] = loadBitmap("monty_tile0001.img");
    monty[2] = loadBitmap("monty_tile0002.img");
    monty[3] = loadBitmap("monty_tile0003.img");
#endif

}

void CreditsScreen_initialPaintCallback(void) {
}

void CreditsScreen_repaintCallback(void) {
    int lines = 1;
    int optionsHeight = 8 * (CreditsScreen_optionsCount);
    size_t len = max(strlen("Options"), strlen(CreditsScreen_options[0]));

    /* The background */
    fillRect(0, 0, (XRES_FRAMEBUFFER - 1), (YRES_FRAMEBUFFER - 1), getPaletteEntry(0xFF6cb1a3), FALSE);

    if (mainText != NULL) {
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 1, lines + 3, "Credits", mainText);
    }

#ifndef TILED_BITMAPS
    drawImageWindow(2, (YRES_FRAMEBUFFER - 72) / 8, monty->width / 8, (monty->height / 8) + 1, "Monty", monty);
#else
    drawWindow(2, (YRES_FRAMEBUFFER - 72) / 8, 2 * (monty[0]->width / 8), 2 * (monty[0]->height / 8) + 1, "Monty" );
    drawBitmap(8, 128, monty[0], TRUE);
    drawBitmap(40, 128, monty[1], TRUE);
    drawBitmap(8, 160, monty[2], TRUE);
    drawBitmap(40, 160, monty[3], TRUE);
#endif

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - len - 3,
                          ((YRES_FRAMEBUFFER / 8) + 1) - (optionsHeight / 8) - 3,
                          len + 2,
                          (optionsHeight / 8) + 2, "", &CreditsScreen_options, CreditsScreen_optionsCount, cursorPosition);
}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, long delta) {
    switch (cmd) {
        case kCommandBack:
            return kMainMenu;
        case kCommandUp:
            --cursorPosition;
            break;
        case kCommandDown:
            ++cursorPosition;
            break;
        case kCommandFire1:
        case kCommandFire2:
        case kCommandFire3:
            return CreditsScreen_nextStateNavigation[cursorPosition];
    }

    if (cursorPosition >= CreditsScreen_optionsCount) {
        cursorPosition = CreditsScreen_optionsCount - 1;
    }

    if (cursorPosition < 0) {
        cursorPosition = 0;
    }

    return kResumeCurrentState;
}

void CreditsScreen_unloadStateCallback(enum EGameMenuState newState) {
#ifndef TILED_BITMAPS
    releaseBitmap(monty);
    monty = NULL;
#else
    releaseBitmap(monty[0]);
    releaseBitmap(monty[1]);
    releaseBitmap(monty[2]);
    releaseBitmap(monty[3]);
#endif
}
