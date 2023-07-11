#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include "TMS9918.h"
#include "YM2413.h"

char getch(void);

extern const struct Pattern patterns[127];

extern uint8_t playerLocation;

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

uint8_t cursorPosition = 0;

uint8_t updateDirection;

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

#define MARGIN_TEXT_SCREEN_LIMIT 35

void init(void) {
    initTMS9918();
    initYM2413();
    cooldown = COOLDOWN_MAX;
}

void writeStr(uint8_t _x, uint8_t y, const char *text) {
    writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}

void showMessage(const char *message) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, message);
    writeStr(2, 22, "Press SPACE button to continue");

    while (keepGoing) {
        if (getKey() == ' ') {
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
    writeStr(1, 6, " Press SPACE to start ");

    while (keepGoing) {
        if (getKey() == ' ') {
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
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

uint8_t getKey(void) {
    uint8_t input = getch();

    performAction();

    switch (input) {
        case 30:
            return 'w';
        case 31:
            return 's';
        case 29:
            updateDirection = 1;
            return 'q';
        case 28:
            updateDirection = 1;
            return 'e';
        case 'z':
            return 'a';

        case 'x':
            return 'd';

    }
    return input;
}

void graphicsFlush(void) {
    flush3DBuffer();

    if (updateDirection) {
        updateDirection = 0;
        switch (getPlayerDirection()) {
            case 0:
                writeStrWithLimit(29, 14, "N", 31);
                break;
            case 1:
                writeStrWithLimit(29, 14, "E", 31);
                break;
            case 2:
                writeStrWithLimit(29, 14, "S", 31);
                break;
            case 3:
                writeStrWithLimit(29, 14, "W", 31);
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
                        realPut((x * 2) + 136 + cx, (y * 2) + cy + 8);
                    }
                }
            }
        }
    }
}

void HUD_initialPaint(void) {
    struct Room *room = getRoom(getPlayerRoom());

    drawLine(128, 0, 128, 191);
    drawLine(0, 128, 255, 128);
    drawMap();

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(17, 17 + i, menuItems[i]);
    }

    writeStrWithLimit(17, 14, "Direction: ", 31);
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

    writeStrWithLimit(1, 17, "Object in room", 16);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, 18, "*", 16);
        }

        writeStrWithLimit(2, 18, item->name, 16);
    } else {
        writeStrWithLimit(2, 18, "Nothing", 16);
    }

    writeStrWithLimit(1, 20, "Object in hand", 16);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(1, 21, "*");
        }

        writeStrWithLimit(2, 21, item->name, 16);
    } else {
        writeStrWithLimit(2, 21, "Nothing", 16);
    }
}