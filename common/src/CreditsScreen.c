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

extern int8_t cursorPosition;
extern const char *mainText;
const char *CreditsScreen_options[1] = {"Back"};

enum EGameMenuState CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

void CreditsScreen_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
    mainText = "Made by MontyOnTheRun";
}

void CreditsScreen_repaintCallback(void) {
    if (firstFrameOnCurrentState) {
        clearScreen();
        if (mainText != NULL) {
            drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, 1 + 3, "Credits", mainText);
        }
    }



    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - 7 - 3,
                          ((YRES_FRAMEBUFFER / 8) + 1) - (1) - 4,
                          7 + 2,
                          3, "Credits", CreditsScreen_options, 1, cursorPosition);
}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, long delta) {
    (void)delta;
    return handleCursor(CreditsScreen_nextStateNavigation, 1, cmd, kMainMenu);
}

void CreditsScreen_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}
