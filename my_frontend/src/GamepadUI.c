/*
   Created by monty on 18-07-2023.
*/
#include <stdint.h>

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

    if (firstFrameOnCurrentState) {
        drawWindow((XRES_FRAMEBUFFER / 8) / 2,
                   0,
                   (XRES_FRAMEBUFFER / 8) / 2 - 1,
                   (YRES_FRAMEBUFFER / 8) / 2 + 2,
                   "Map",
                   2);
    }

    drawWindowWithOptions(
            1 + (XRES_FRAMEBUFFER / 2) / 8,
            (YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6 - 1,
            (XRES_FRAMEBUFFER / 8) - (1 + (XRES_FRAMEBUFFER / 2) / 8) - 1,
            (YRES_FRAMEBUFFER / 8) - ((YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6 - 1) - 1,
            "Actions",
            menuItems,
            6,
            cursorPosition);

    HUD_refresh();
}
