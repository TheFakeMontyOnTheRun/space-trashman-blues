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

extern const char *mainText;
extern int8_t cursorPosition;

const char *HelpScreen_options[1] = {"Back"};

const enum EGameMenuState HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

void HelpScreen_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
    mainText = "You must gain access to the reactor, place the EMP device and, from a safe distance, activate it.";
}

void HelpScreen_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();

#ifndef MONOCHROME_VECTORS
        fillRect(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF00FF00), 0);
#endif

        if (mainText != NULL) {
            drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, 1 + 6, "Help", mainText);
        }
    }

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - 4 - 4,
                          ((YRES_FRAMEBUFFER / 8) + 1) - 1 - 5,
                          4 + 2,
                          1 + 2, "Help", HelpScreen_options, 1, cursorPosition);
}

enum EGameMenuState HelpScreen_tickCallback(enum ECommand cmd, void* data) {
    (void)data;
    return handleCursor(HelpScreen_nextStateNavigation, 1, cmd, kMainMenu);
}

void HelpScreen_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}
