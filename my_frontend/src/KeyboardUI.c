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
    drawLine(128, 0, 128, 191, 2);
    drawLine(0, 128, 255, 128, 2);
    drawMap();

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(17, 17 + i, menuItems[i]);
    }

    writeStrWithLimit(17, 14, "Direction: ", 31, 2, 0);
    HUD_refresh();
}

void HUD_refresh(void) {

    for (uint8_t d = 0; d < 15; ++d) {
        writeStr(1 + d, YRES_TEXT - 3, " ");
        writeStr(1 + d, YRES_TEXT - 4, " ");
        writeStr(1 + d, YRES_TEXT - 6, " ");
        writeStr(1 + d, YRES_TEXT - 7, " ");
    }

    writeStrWithLimit(1, YRES_TEXT - 7, "Object in room", 16, 2, 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, YRES_TEXT - 6, "*", 16, 2, 0);
        }

        writeStrWithLimit(2, YRES_TEXT - 6, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 6, "Nothing", 16, 2, 0);
    }

    writeStrWithLimit(1, YRES_TEXT - 4, "Object in hand", 16, 2, 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);

        if (item->active) {
            writeStr(1, YRES_TEXT - 3, "*");
        }

        writeStrWithLimit(2, YRES_TEXT - 3, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, YRES_TEXT - 3, "Nothing", 16, 2, 0);
    }
}