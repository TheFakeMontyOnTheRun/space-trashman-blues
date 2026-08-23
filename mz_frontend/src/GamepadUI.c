/*
   Created by monty on 18-07-2023.
*/
#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#else
#include <genesis.h>
#endif
#endif

#include "Enums.h"
#include "Renderer.h"
#include "UI.h"
#include "GamepadUI.h"

extern uint8_t firstFrameOnCurrentState;

const char *menuItems[] = {
        "Use/Toggle",
        "Use with",
        "Use/pick",
        "Drop",
        "Next(item)",
        "Next(room)",
};

void initGamepadUI(void) {
    cursorPosition = 0;
}

enum ECommand performActionJoypad(void) {
    return kCommandFire1 + cursorPosition;
}

void HUD_initialPaint(void) {

    drawWindowWithOptions(
            1 + (XRES_FRAMEBUFFER / 2) / 8,
            (YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 5,
            (XRES_FRAMEBUFFER / 8) - (1 + (XRES_FRAMEBUFFER / 2) / 8) - 1,
            (YRES_FRAMEBUFFER / 8) - ((YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6 - 1) - 3,
            "Actions",
            menuItems,
            6,
            cursorPosition);
}
