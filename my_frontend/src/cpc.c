#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <games.h>
#include <psg.h>
#include <sound.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

uint8_t cursorPosition = 0;
extern uint8_t playerLocation;

#define BUFFER_SIZEX 32
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 40

uint8_t font[] = {
        // ASCII table starting on SPACE.
        // Being on line 32 is no accident.
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // space
        , 0x10, 0x38, 0x38, 0x10, 0x10, 0x00, 0x10, 0x00, 0x6c, 0x6c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
        0x7c, 0x28, 0x28, 0x7c, 0x28, 0x00, 0x20, 0x38, 0x40, 0x30, 0x08, 0x70, 0x10, 0x00, 0x64, 0x64, 0x08, 0x10,
        0x20, 0x4c, 0x4c, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00,
        0x00, 0x28, 0x38, 0x7c, 0x38, 0x28, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x30, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00 // /space - 15
        , 0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x38, 0x00 // 0
        , 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x38, 0x44, 0x04, 0x18, 0x20, 0x40, 0x7c, 0x00, 0x38, 0x44,
        0x04, 0x38, 0x04, 0x44, 0x38, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x04, 0x44, 0x38, 0x00, 0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x04, 0x08, 0x10, 0x20, 0x20,
        0x20, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x38, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30, 0x00,
        0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x08, 0x10,
        0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04,
        0x08, 0x10, 0x20, 0x00, 0x38, 0x44, 0x04, 0x18, 0x10, 0x00, 0x10, 0x00, 0x38, 0x44, 0x5c, 0x54, 0x5c, 0x40,
        0x38, 0x00 // /0
        , 0x38, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00 // a
        , 0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00, 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 0x78, 0x44,
        0x44, 0x44, 0x44, 0x44, 0x78, 0x00, 0x7c, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x40, 0x5c, 0x44, 0x44, 0x3c, 0x00, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44,
        0x44, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00,
        0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7c, 0x00, 0x44, 0x6c,
        0x54, 0x44, 0x44, 0x44, 0x44, 0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x44, 0x44, 0x00, 0x38, 0x44, 0x44, 0x44,
        0x44, 0x44, 0x38, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x44, 0x44, 0x54, 0x48,
        0x34, 0x00, 0x78, 0x44, 0x44, 0x78, 0x48, 0x44, 0x44, 0x00, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00,
        0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x44, 0x44,
        0x44, 0x44, 0x44, 0x28, 0x10, 0x00, 0x44, 0x44, 0x54, 0x54, 0x54, 0x54, 0x28, 0x00, 0x44, 0x44, 0x28, 0x10,
        0x28, 0x44, 0x44, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00, 0x78, 0x08, 0x10, 0x20, 0x40, 0x40,
        0x78, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
        0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x08,
        0x30, 0x40, 0x78, 0x00, 0x18, 0x20, 0x20, 0x60, 0x20, 0x20, 0x18, 0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10,
        0x10, 0x00, 0x30, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x30, 0x00, 0x28, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x38, 0x6c, 0x44, 0x44, 0x7c, 0x00, 0x00
};

int getch();

const uint16_t lineStart[128] = {
        49232l,
        51280l,
        53328l,
        55376l,
        57424l,
        59472l,
        61520l,
        63568l,
        49312l,
        51360l,
        53408l,
        55456l,
        57504l,
        59552l,
        61600l,
        63648l,
        49392l,
        51440l,
        53488l,
        55536l,
        57584l,
        59632l,
        61680l,
        63728l,
        49472l,
        51520l,
        53568l,
        55616l,
        57664l,
        59712l,
        61760l,
        63808l,
        49552l,
        51600l,
        53648l,
        55696l,
        57744l,
        59792l,
        61840l,
        63888l,
        49632l,
        51680l,
        53728l,
        55776l,
        57824l,
        59872l,
        61920l,
        63968l,
        49712l,
        51760l,
        53808l,
        55856l,
        57904l,
        59952l,
        62000l,
        64048l,
        49792l,
        51840l,
        53888l,
        55936l,
        57984l,
        60032l,
        62080l,
        64128l,
        49872l,
        51920l,
        53968l,
        56016l,
        58064l,
        60112l,
        62160l,
        64208l,
        49952l,
        52000l,
        54048l,
        56096l,
        58144l,
        60192l,
        62240l,
        64288l,
        50032l,
        52080l,
        54128l,
        56176l,
        58224l,
        60272l,
        62320l,
        64368l,
        50112l,
        52160l,
        54208l,
        56256l,
        58304l,
        60352l,
        62400l,
        64448l,
        50192l,
        52240l,
        54288l,
        56336l,
        58384l,
        60432l,
        62480l,
        64528l,
        50272l,
        52320l,
        54368l,
        56416l,
        58464l,
        60512l,
        62560l,
        64608l,
        50352l,
        52400l,
        54448l,
        56496l,
        58544l,
        60592l,
        62640l,
        64688l,
        50432l,
        52480l,
        54528l,
        56576l,
        58624l,
        60672l,
        62720l,
        64768l
};

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

void backToGraphics();

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr);

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint16_t cooldown;

void init() {
    cooldown = COOLDOWN_MAX;
}

char *menuItems[] = {
        "8) Use/Toggle",
        "5) Use with...",
        "9) Use/pick...",
        "6) Drop",
        "7) Next item",
        "4) Next in room",
};

void graphicsFlush();

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX) {

    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t x = _x;

    uint8_t *lineBase = (unsigned char *) 0xC000 + ((((y * 8)) / 8) * 80) + ((((y * 8)) & 7) * 2048);

    for (; c < len && y < 64; ++c) {

        char cha = *ptr;

        if (x == limitX) {
            ++y;
            lineBase += 2048;
            x = _x;
        } else if (cha == '\n') {
            ++y;
            lineBase += 2048;
            x = _x;
            ++ptr;
            continue;
        }

        if (cha >= 'a') {
            if (cha <= 'z') {
                cha = (cha - 'a') + 'A';
            } else {
                cha -= ('z' - 'a');
            }
        }

        uint8_t *fontTop = &font[((cha - 32) << 3)];

        uint8_t *line = lineBase + 2 * x + 1;

        for (int d = 0; d < 8; ++d) {
            int e;
            uint8_t chunk = *fontTop;
            uint8_t *pixel = line;

            *pixel = 0;

            for (e = 0; e < 4; ++e) {
                if (chunk & 1) {
                    *pixel |= (16 << e);
                }
                chunk = chunk >> 1;
            }

            --pixel;

            *pixel = 0;

            for (e = 0; e < 4; ++e) {
                if (chunk & 1) {
                    *pixel |= (16 << e);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
            line += 2048;
        }

        ++x;
        ++ptr;
    }
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

uint8_t *realPut(int x, int y, uint8_t colour, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = (unsigned char *) 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048) + (x >> 2);
    }
    *ptr |= (8 >> (x & 3));

    return ptr;
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {

    int x, y;
    uint8_t *ptr;

    for (x = 0; x < tw * 8; ++x) {
        if ((x & 3) == 0) {
            ptr = NULL;
        }
        ptr = realPut(((tx * 8) + x), (ty * 8), 1, ptr);
    }

    for (x = 0; x < tw * 8; ++x) {
        if ((x & 3) == 0) {
            ptr = NULL;
        }
        ptr = realPut(((tx * 8) + x), (ty + th) * 8, 1, ptr);
    }

    for (y = 0; y < th * 8; ++y) {
        realPut((tx + tw) * 8, ((ty * 8) + y), 1, NULL);
    }

    for (y = 0; y < th * 8; ++y) {
        realPut((tx * 8), ((ty * 8) + y), 1, NULL);
    }

    writeStr(tx + 1, ty, title, 0, 0);
}


void showMessage(const char *message) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, message, 2, 0);
    writeStr(2, 22, "Press SPACE button to continue", 2, 0);

    while (keepGoing) {
        if (getKey() == ' ') {
            keepGoing = 0;
        }
    }

    backToGraphics();
}

void titleScreen() {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, "Sub Mare Imperium: Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 6, " Press SPACE to start ", 2, 0);

    while (keepGoing) {
        if (getKey() == ' ') {
            keepGoing = 0;
        }
    }
    backToGraphics();
}

void refreshJustGraphics() {
    clearGraphics();
    renderScene();
    graphicsFlush();
}

void backToGraphics() {
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

void clearTextScreen() {
    clearScreen();
}

void enterTextMode() {
}

void exitTextMode() {
}

void drawMap() {

    uint8_t *ptr;

    if (playerLocation == 0) {
        return;
    }

    memset(buffer, 0, BUFFER_SIZEX * BUFFER_SIZEY);

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {

            if ((x & 7) == 0) {
                ptr = NULL;
            }

            uint8_t newCell = map[y][x];
            newCell = newCell & 127;
            uint8_t block = patterns[newCell - 32].blockMovement;

            for (int cy = 0; cy < 4; ++cy) {
                ptr = NULL;
                for (int cx = 0; cx < 4; ++cx) {
                    if (block) {
                        ptr = graphicsPutAddr((x * 4) + cx, (y * 4) + cy, ptr);
                    }
                }
            }
        }
    }

    drawWindow(0, 0, 17, 20, "Map");
    graphicsFlush();
}

uint8_t getKey() {
    uint8_t input = getch();

    performAction();

    switch (input) {
        case 30:
            return 'w';
        case 31:
            return 's';
        case 29:
            return 'q';
        case 28:
            return 'e';
        case 'z':
            return 'a';

        case 'x':
            return 'd';
        case 'm':
            drawMap();
            return '.';
    }
    return input;
}

void shutdownGraphics() {
}

void clearScreen() {
    memset((unsigned char *) 0xC000, 0, (320 / 4) * 200);
}

void clearGraphics() {
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void graphicsFlush() {

    for (int y = 0; y < BUFFER_SIZEY; ++y) {
        uint8_t *line = (unsigned char *) lineStart[y];
        memcpy(line + 1, buffer + (y * BUFFER_SIZEX), BUFFER_SIZEX);
    }

    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);

}

void sleepForMS(uint32_t ms) {
}


void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t _y0 = y0;
    uint8_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    uint8_t *ptr = &buffer[(_y0 * (BUFFER_SIZEX)) + (x0 / 4)]; //skip to the line in pattern
    uint8_t y;
    for (y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            *ptr |= (8 >> (x0 & 3));
        }
        ptr += BUFFER_SIZEX;
    }

}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 4)]; //skip to the line in pattern
    }

    *ptr |= (8 >> (x & 3));

    return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {
    buffer[(y * (BUFFER_SIZEX)) + (x / 4)] |= (8 >> (x & 3));
}

void HUD_initialPaint() {
    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(19, 11 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();

    drawWindow(0, 0, 17, 20, "3D View");
    drawWindow(18, 0, 21, 20, "Controls and status");
}

void HUD_refresh() {

    writeStrWithLimit(19, 7, "Object in hand", MARGIN_TEXT_SCREEN_LIMIT);

    for (uint8_t d = 0; d < 19; ++d) {
        writeStr(20 + d, 4, " ", 2, 0);
        writeStr(20 + d, 5, " ", 2, 0);
        writeStr(20 + d, 8, " ", 2, 0);
        writeStr(20 + d, 9, " ", 2, 0);
    }


    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(19, 8, "*", 2, 0);
        } else {
            writeStr(19, 8, " ", 2, 0);
        }

        writeStrWithLimit(20, 8, item->name, MARGIN_TEXT_SCREEN_LIMIT);
    } else {
        writeStrWithLimit(20, 8, " Nothing", MARGIN_TEXT_SCREEN_LIMIT);
    }

    writeStrWithLimit(19, 3, "Object in room", MARGIN_TEXT_SCREEN_LIMIT);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStr(19, 4, "*", 2, 0);
        } else {
            writeStr(19, 4, " ", 2, 0);
        }

        writeStrWithLimit(20, 4, item->name, MARGIN_TEXT_SCREEN_LIMIT);
    } else {
        writeStrWithLimit(19, 4, " Nothing", MARGIN_TEXT_SCREEN_LIMIT);
    }
}

