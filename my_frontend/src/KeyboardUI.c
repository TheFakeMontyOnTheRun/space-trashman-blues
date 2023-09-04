/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdint.h>
#include <stdlib.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "KeyboardUI.h"

#define XRES_TEXT (XRES_FRAMEBUFFER / 8)
#define YRES_TEXT (YRES_FRAMEBUFFER / 8)

uint8_t cursorPosition = 0;

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

char getch(void);

char *menuItems[] = {
        "8) Use/Toggle",
        "5) Use with...",
        "9) Use/pick...",
        "6) Drop",
        "7) Next item",
        "4) Next in room",
};

#define COOLDOWN_MAX 0x2EF
uint16_t cooldown;

void initKeyboardUI(void) {
    cooldown = COOLDOWN_MAX;
}


void HUD_initialPaint(void) {
	uint8_t i;
    drawLine(128, 0, 128, 128, 2);
    drawMap();

    for (i = 0; i < 6; ++i) {
        writeStr(17, (YRES_TEXT - 7) + i, menuItems[i]);
    }

    writeStrWithLimit(1, YRES_TEXT - 7, "Direction: ", 31, 2, 0);
    HUD_refresh();
}

void HUD_refresh(void) {
	uint8_t d;
    for (d = 0; d < 15; ++d) {
        writeStr(1 + d, YRES_TEXT - 2, " ");
        writeStr(1 + d, YRES_TEXT - 3, " ");
        writeStr(1 + d, YRES_TEXT - 5, " ");
        writeStr(1 + d, YRES_TEXT - 6, " ");
    }

    writeStrWithLimit(1, YRES_TEXT - 6, "Object in room", 16, 2, 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, YRES_TEXT - 5, "*", 16, 2, 0);
        }

        writeStrWithLimit(2, YRES_TEXT - 5, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 5, "Nothing", 16, 2, 0);
    }

    writeStrWithLimit(1, YRES_TEXT - 3, "Object in hand", 16, 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            writeStr(1, YRES_TEXT - 2, "*");
        }

        writeStrWithLimit(2, YRES_TEXT - 2, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 2, "Nothing", 16, 2, 0);
    }
}
