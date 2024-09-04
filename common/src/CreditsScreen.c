#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include <stdlib.h>

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "Engine.h"

#ifndef MONOCHROME_VECTORS
#include "FixP.h"
#include "Vec.h"
#include "Mesh.h"
#include "CActor.h"
#include "Renderer.h"
#endif

extern int8_t cursorPosition;
extern const char *mainText;
const char *CreditsScreen_options[1] = {"Back"};

enum EGameMenuState CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

void CreditsScreen_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
    mainText = "Made by Daniel \"MontyOnTheRun\" Monteiro, with the help of many  people. Please check CREDITS.TXT"
               " on the source code tree for further acknowledgements.";
}

void CreditsScreen_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();

#ifndef MONOCHROME_VECTORS
        fillRect(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF00FF00), 0);
#endif

        if (mainText != NULL) {
            drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 4, 10, "Credits", mainText);
        }
    }

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - 7 - 4,
                          ((YRES_FRAMEBUFFER / 8) + 1) - (1) - 5,
                          7 + 2,
                          3, "Credits", CreditsScreen_options, 1, cursorPosition);
}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, void* data) {
    (void)data;
    return handleCursor(CreditsScreen_nextStateNavigation, 1, cmd, kMainMenu);
}

void CreditsScreen_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}
