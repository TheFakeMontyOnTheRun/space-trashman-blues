/*
   Created by monty on 18-07-2023.
*/
#include <stdint.h>
#include <stdlib.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "GamepadUI.h"

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

uint8_t cursorPosition = 0;

void initGamepadUI(void) {
    cursorPosition = 0;
}

void performActionJoypad(void) {
    performAction();

/*
char *menuItems[] = {
 0       "Use/Toggle current item",
 1       "Use current item with...",
 2       "Pick",
 3       "Drop",
 4       "Next item in inventory",
 5       "Next room item in focus",
};
*/
    switch (cursorPosition) {
        case 0:
            useObjectNamed(getItem(focusedItem->item)->name);
            break;
        case 1:
            interactWithItemInRoom();
            HUD_refresh();
            break;
        case 2:
            pickItem();
            refreshJustGraphics();
            HUD_refresh();
            break;
        case 3:
            dropItem();
            refreshJustGraphics();
            HUD_refresh();
            break;
        case 4:
            nextItemInHand();
            break;
        case 5:
            nextItemInRoom();
            break;
    }
}

void HUD_initialPaint(void) {
    drawLine(128, 0, 128, 128, 2);
    drawMap();

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(18, 17 + i, menuItems[i]);
    }

    writeStrWithLimit(1, 17, "Direction: ", 31, 2, 0);
    HUD_refresh();
}

void HUD_refresh(void) {

    for (uint8_t d = 0; d < 15; ++d) {
        writeStr(1 + d, 18, " ");
        writeStr(1 + d, 19, " ");
        writeStr(1 + d, 22, " ");
        writeStr(1 + d, 23, " ");
    }


    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(17, 17 + i, (i == cursorPosition) ? ">" : " ");
    }

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
            writeStr(1, 22, "*");
        }

        writeStrWithLimit(2, 22, item->name, 16, 2, 0);
    } else {
        writeStrWithLimit(2, 22, "Nothing", 16, 2, 0);
    }
}
