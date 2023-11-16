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

char getch(void);

const char *menuItems[] = {
        "8) Use",
        "5) Use with",
        "9) Pick",
        "6) Drop",
        "7) Next(hand)",
        "4) Next(room)",
};

#define COOLDOWN_MAX 0x2EF
uint16_t cooldown;

void initKeyboardUI(void) {
    cooldown = COOLDOWN_MAX;
}


void HUD_initialPaint(void) {
    drawLine(128, 0, 128, 128, 2);
    drawMap();
    HUD_refresh();
}

void HUD_refresh(void) {
	uint8_t d, e;
    /*
    for (d = 0; d < 15; ++d) {
        for (e = 2; e < 6; ++e) {
            drawTextAt(1 + d, YRES_TEXT - d, " ", 1);
        }
    }
     */

    writeStrWithLimit(1, YRES_TEXT - 6, "In room", 16, 2, 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);

        if (item->active) {
            writeStrWithLimit(1, YRES_TEXT - 5, "*", 16, 2, 0);
        }

        writeStrWithLimit(2, YRES_TEXT - 5, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 5, "-", 16, 2, 0);
    }

    writeStrWithLimit(1, YRES_TEXT - 3, "In hand", 16, 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            drawTextAt(1, YRES_TEXT - 2, "*", 1);
        }

        writeStrWithLimit(2, YRES_TEXT - 2, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 2, "-", 16, 2, 0);
    }
}
