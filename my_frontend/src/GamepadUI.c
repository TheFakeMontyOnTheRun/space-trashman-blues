/*
   Created by monty on 18-07-2023.
*/
#include <stdint.h>
#include <stdlib.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "GamepadUI.h"

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

char *menuItems[] = {
        "Use/Toggle",
        "Use with",
        "Use/pick",
        "Drop",
        "Next item",
        "Next in room",
};

extern int cursorPosition;

void initGamepadUI(void) {
    cursorPosition = 0;
}

enum ECommand performActionJoypad(void) {
    return kCommandFire1 + cursorPosition;
}

void HUD_initialPaint(void) {
    drawLine(128, 0, 128, 128, 2);
    drawMap();
    writeStrWithLimit(1, 17, "Direction: ", 31, 2, 0);
    HUD_refresh();
}

void HUD_refresh(void) {

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) /*biggestOption*/ 12 - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6,
            12/*biggestOption*/ + 2,
            6 /*kMainMenuOptionsCount*/ + 2,
            "Actions",
            menuItems,
            6,
            cursorPosition);

    writeStrWithLimit(1, 18, "Object in room", 16, 2, 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, 19, "*", 16, 2, 0);
        }

        writeStrWithLimit(2, 19, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, 19, "Nothing", 16, 2, 0);
    }

    writeStrWithLimit(1, 21, "Object in hand", 16, 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            drawTextAt(1, 22, "*", 1);
        }

        writeStrWithLimit(2, 22, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, 22, "Nothing", 16, 2, 0);
    }
}
