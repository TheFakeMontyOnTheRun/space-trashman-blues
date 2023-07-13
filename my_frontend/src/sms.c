#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sms.h>
#include <stdio.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include "TMS9918.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern uint8_t playerLocation;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

#define COOLDOWN_MAX 0xFF
#define MARGIN_TEXT_SCREEN_LIMIT 30

uint8_t cursorPosition = 0;

uint8_t updateDirection;
uint8_t cooldown;

void init(void) {
    initTMS9918();
    cooldown = COOLDOWN_MAX;
}

char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

void writeStr(uint8_t _x, uint8_t y, const char *text) {
    writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT, 2, 0);
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}

void showMessage(const char *message) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, message);
    writeStr(2, 22, "Press B button to continue");

    while (keepGoing) {
        if (getKey() == 'p') {
            keepGoing = 0;
        }
    }

    backToGraphics();
}

void titleScreen(void) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, "Sub Mare Imperium: Derelict");
    writeStr(1, 4, "by Daniel Monteiro");
    writeStr(1, 6, " Press B to start ");

    while (keepGoing) {
        if (getKey() == 'p') {
            keepGoing = 0;
        }
    }
    backToGraphics();
}

void refreshJustGraphics(void) {
    clearGraphics();
    renderScene();
    graphicsFlush();
}

void backToGraphics(void) {
    clearScreen();
    HUD_initialPaint();
    refreshJustGraphics();
}

void performAction(void) {
    switch (getGameStatus()) {
        case kBadVictory:
            showMessage("Victory! Too bad you didn't survive");
            while (1);

        case kBadGameOver:
            showMessage("You're dead! And so are the\n"
                        "other people on the path of\n"
                        "destruction faulty reactor");
            while (1);

        case kGoodVictory:
            showMessage("Victory! You managed to destroy the\nship and get out alive");
            while (1);

        case kGoodGameOver:
            showMessage("You failed! While you're alive\n"
                        "you failed to prevent the worst\n"
                        "scenario and now EVERYBODY is\n"
                        "dead!)");
            while (1);

        default:
        case kNormalGameplay:
            break;
    }

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

uint8_t getKey(void) {
    int key = read_joypad1();

    if (cooldown) {
        cooldown--;
    }

    if (key & JOY_UP) {
        return 'w';
    }

    if (key & JOY_LEFT) {
        if (key & JOY_FIREB) {
            return 'a';
        } else {
            updateDirection = 1;
            return 'q';
        }
    }

    if (key & JOY_RIGHT) {
        if (key & JOY_FIREB) {
            return 'd';
        } else {
            updateDirection = 1;
            return 'e';
        }
    }

    if (key & JOY_DOWN) {
        return 's';
    }

    if ((key & JOY_FIREA) && !cooldown) {
        performAction();
        cooldown = COOLDOWN_MAX;
        return 'p';
    }

    if ((key & JOY_FIREB) && !cooldown) {
        cursorPosition = (cursorPosition + 1);

        if (cursorPosition >= 6) {
            cursorPosition = 0;
        }

        HUD_refresh();
        cooldown = COOLDOWN_MAX;
        return 'p';
    }

    return '.';
}

void graphicsFlush(void) {
    flush3DBuffer();

    if (updateDirection) {
        updateDirection = 0;
        switch (getPlayerDirection()) {
            case 0:
                writeStrWithLimit(12, 17, "N", 31, 2, 0);
                break;
            case 1:
                writeStrWithLimit(12, 17, "E", 31,2 , 0);
                break;
            case 2:
                writeStrWithLimit(12, 17, "S", 31, 2, 0);
                break;
            case 3:
                writeStrWithLimit(12, 17, "W", 31, 2, 0);
                break;
        }
    }
}

void drawMap(void) {

    uint8_t x, y;

    if (playerLocation == 0) {
        return;
    }

    for (y = 0; y < 8; ++y) {
        writeStr(17, 1 + y, "         ");
    }

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                for (int cy = 0; cy < 2; ++cy) {
                    for (int cx = 0; cx < 2; ++cx) {
                        realPut((x * 2) + 136 + cx, (y * 2) + cy + 8, 1 , NULL);
                    }
                }
            }
        }
    }
}

void HUD_initialPaint(void) {
    drawLine(128, 0, 128, 128, 2);
    drawLine(0, 128, 255, 128, 2);
    drawMap();

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(18, 17 + i, menuItems[i]);
    }

    writeStrWithLimit(1, 17, "Direction: ", 31,2 , 0);
    updateDirection = 1;
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

    writeStrWithLimit(1, 18, "Object in room", 16, 2 , 0);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, 19, "*", 16, 2 , 0);
        }

        writeStrWithLimit(2, 19, item->name, 16, 2 , 0);
    } else {
        writeStrWithLimit(2, 19, "Nothing", 16, 2 , 0);
    }

    writeStrWithLimit(1, 21, "Object in hand", 16, 2 , 0);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(1, 22, "*");
        }

        writeStrWithLimit(2, 22, item->name, 16, 2 , 0);
    } else {
        writeStrWithLimit(2, 22, "Nothing", 16, 2 , 0);
    }
}
