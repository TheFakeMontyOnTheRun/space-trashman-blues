/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdint.h>
#include <stdlib.h>

#include "Common.h"
#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t firstFrameOnCurrentState;

const char *menuItems[] = {
        "1) Use",
        "2) Use with",
        "3) Pick",
        "4) Drop",
        "5) Next(hand)",
        "6) Next(room)",
};

void initKeyboardUI(void) {
}


void HUD_initialPaint(void) {

    drawWindowWithOptions(
            1 + (XRES_FRAMEBUFFER / 2) / 8,
            (YRES_FRAMEBUFFER / 8) - 3 - 5,
            (XRES_FRAMEBUFFER / 8) - (1 + (XRES_FRAMEBUFFER / 2) / 8) - 1,
            (YRES_FRAMEBUFFER / 8) - ((YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6 - 1) - 2,
            "Actions",
            menuItems,
            6,
            0xFF);
}

