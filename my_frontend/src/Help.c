/*
   Created by monty on 01-10-2023.
*/
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include "Enums.h"
#include "UI.h"
#include "Renderer.h"
#include "Engine.h"

extern const char *mainText;
extern int8_t cursorPosition;

const char *HelpScreen_options[1] = {"Back"};

const enum EGameMenuState HelpScreen_nextStateNavigation[1] = {
        kMainMenu,
};

void HelpScreen_initStateCallback(enum EGameMenuState tag) {
    mainText = "This is the help";
}

void HelpScreen_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, 1 + 3, "Help", mainText);
    }

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - 4 - 3,
                          ((YRES_FRAMEBUFFER / 8) + 1) - 1 - 4,
                          4 + 2,
                          1 + 2, "Help", HelpScreen_options, 1, cursorPosition);
}

enum EGameMenuState HelpScreen_tickCallback(enum ECommand cmd, long delta) {
    return handleCursor(HelpScreen_nextStateNavigation, 1, cmd, kMainMenu);
}

void HelpScreen_unloadStateCallback(enum EGameMenuState newState) {
}
