/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"

extern int cursorPosition;
extern const char *mainText;
const char *CreditsScreen_options[1] = {"Back"};

int32_t CreditsScreen_nextStateNavigation[1] = {
        kMainMenu,
};

void CreditsScreen_initStateCallback(int32_t tag) {
    mainText = "Made by MontyOnTheRun";
}

void CreditsScreen_initialPaintCallback(void) {
    clearScreen();
}

void CreditsScreen_repaintCallback(void) {
    if (mainText != NULL) {
        drawTextWindow(1, 1, (XRES_FRAMEBUFFER / 8) - 2, 1 + 3, "Credits", mainText);
    }

    drawWindowWithOptions((XRES_FRAMEBUFFER / 8) - 7 - 3,
                          ((YRES_FRAMEBUFFER / 8) + 1) - (1) - 3,
                          7 + 2,
                          2, "", CreditsScreen_options, 1, cursorPosition);
}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, long delta) {
    return handleCursor(CreditsScreen_nextStateNavigation, 1, cmd, kMainMenu);
}

void CreditsScreen_unloadStateCallback(int32_t newState) {
}
