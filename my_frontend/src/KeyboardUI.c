/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdint.h>
#include <stdlib.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;


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
    drawWindow((XRES_FRAMEBUFFER / 8) / 2,
               0,
               (XRES_FRAMEBUFFER / 8) / 2 - 1,
               (YRES_FRAMEBUFFER / 8) / 2 + 2,
               "Map",
               2);
    drawMap();

    drawWindow(0,
               YRES_TEXT - 8,
               (XRES_FRAMEBUFFER / 8) / 2,
               7,
               "Direction: ",
               2);
    HUD_refresh();

    drawWindowWithOptions(
            (XRES_FRAMEBUFFER / 8) - (int) /*biggestOption*/ 12 - 3,
            (YRES_FRAMEBUFFER / 8) - 3 - /*kMainMenuOptionsCount*/ 6,
            12/*biggestOption*/ + 2,
            6 /*kMainMenuOptionsCount*/ + 2,
            "Actions",
            menuItems,
            6,
            -1);
}

void HUD_refresh(void) {
	uint8_t d, e;

    for (d = 0; d < 15; ++d) {
        for (e = 2; e < 6; ++e) {
            drawTextAt(1 + d, YRES_TEXT - e, " ", 1);
        }
    }

    writeStrWithLimit(1, YRES_TEXT - 7, "In room", 16, 2, 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStrWithLimit(1, YRES_TEXT - 6, "*", 16, 2, 0);
        }

        writeStrWithLimit(2, YRES_TEXT - 6, item->name, 16, 2, 0);
    }

    writeStrWithLimit(1, YRES_TEXT - 4, "In hand", 16, 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            drawTextAt(1, YRES_TEXT - 3, "*", 1);
        }

        writeStrWithLimit(2, YRES_TEXT - 3, item->name, 16, 2, 0);
    }
}
