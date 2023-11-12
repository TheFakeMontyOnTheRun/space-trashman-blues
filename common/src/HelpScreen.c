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

const char *HelpScreen_options[1] = {"Back"};

const enum EGameMenuState HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

const int16_t HelpScreen_optionsCount = 1;

void HelpScreen_initStateCallback(int32_t tag) {
    mainText = textBuffer;
    memFill(textBuffer, 0, TEXT_BUFFER_SIZE);

#ifndef AGS
    struct StaticBuffer textFile = loadBinaryFileFromPath("Help.txt");
#else
    struct StaticBuffer textFile = loadBinaryFileFromPath("HelpAGS.txt");
#endif

    memCopyToFrom(textBuffer, (void *) textFile.data, textFile.size);
    disposeDiskBuffer(textFile);
}

void HelpScreen_initialPaintCallback(void) {
}

void HelpScreen_repaintCallback(void) {
    int lines;
    int optionsHeight = 8 * (HelpScreen_optionsCount);
    size_t len = max(strlen(&HelpScreen_options[0][0]), strlen("Help"));
    fill(0, 0, XRES_FRAMEBUFFER - 1, YRES_FRAMEBUFFER - 1, getPaletteEntry(0xFF6cb1a3), FALSE);

#ifndef AGS
    lines = 18;
#else
    lines = 2;
#endif

    if (mainText != NULL) {
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, lines + 3, "Help", mainText);
    }

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - len - 3,
               ((YRES_FRAMEBUFFER / 8) + 1) - (optionsHeight / 8) - 3,
               len + 2,
               (optionsHeight / 8) + 2, "", HelpScreen_options, HelpScreen_optionsCount, cursorPosition);

}

enum EGameMenuState HelpScreen_tickCallback(enum ECommand cmd, long delta) {
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
            return HelpScreen_nextStateNavigation[cursorPosition];
    }

    if (cursorPosition >= HelpScreen_optionsCount) {
        cursorPosition = HelpScreen_optionsCount - 1;
    }

    if (cursorPosition < 0) {
        cursorPosition = 0;
    }

    return kResumeCurrentState;
}

void HelpScreen_unloadStateCallback(int32_t newState) {
}
