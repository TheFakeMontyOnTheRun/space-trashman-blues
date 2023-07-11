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
uint8_t updateDirection;


#define BUFFER_SIZEX 32
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0xFF
#define MARGIN_TEXT_SCREEN_LIMIT 40

uint8_t font[] = {
        /*  ASCII table starting on SPACE. */
        /*  Being on line 32 is no accident. */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /*  space */
        , 0x10, 0x38, 0x38, 0x10, 0x10, 0x00, 0x10, 0x00, 0x6c, 0x6c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
        0x7c, 0x28, 0x28, 0x7c, 0x28, 0x00, 0x20, 0x38, 0x40, 0x30, 0x08, 0x70, 0x10, 0x00, 0x64, 0x64, 0x08, 0x10,
        0x20, 0x4c, 0x4c, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00,
        0x00, 0x28, 0x38, 0x7c, 0x38, 0x28, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x30, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00 /*  space - 15 */
        , 0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x38, 0x00 /*  0 */
        , 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x38, 0x44, 0x04, 0x18, 0x20, 0x40, 0x7c, 0x00, 0x38, 0x44,
        0x04, 0x38, 0x04, 0x44, 0x38, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x04, 0x44, 0x38, 0x00, 0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x04, 0x08, 0x10, 0x20, 0x20,
        0x20, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x38, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30, 0x00,
        0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x08, 0x10,
        0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04,
        0x08, 0x10, 0x20, 0x00, 0x38, 0x44, 0x04, 0x18, 0x10, 0x00, 0x10, 0x00, 0x38, 0x44, 0x5c, 0x54, 0x5c, 0x40,
        0x38, 0x00 /*  0 */
        , 0x38, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00 /*  a */
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

int getch(void);

uint16_t lineStart[128];
uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint8_t cooldown;

void init(void) {

    for (int y = 0; y < 128; ++y) {
        lineStart[y] = 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048);
    }

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

void writeStr(uint8_t _x, uint8_t y, const char *text) {
    writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

uint8_t *realPut(int x, int y, uint8_t colour, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = (unsigned char *) 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048) + (x >> 2);
    }

    if (colour) {
        *ptr |= (8 >> (x & 3));
    } else {
        *ptr &= ~(8 >> (x & 3));
    }


    return ptr;
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {

    int x, y;
    uint8_t *ptr = NULL;

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
        ptr = realPut(((tx * 8) + x), ((ty + th) * 8) - 1, 1, ptr);
    }

    for (y = 0; y < (th * 8) - 1; ++y) {
        realPut((tx + tw) * 8, ((ty * 8) + y), 1, NULL);
        realPut((tx * 8), ((ty * 8) + y), 1, NULL);
    }

    writeStr(tx + 1, ty, title);
}


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

void clearTextScreen(void) {
    clearScreen();
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

void drawMap(void) {

    uint8_t *ptr;
    uint8_t x, y;

    if (playerLocation == 0) {
        return;
    }

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                for (int cy = 0; cy < 2; ++cy) {
                    ptr = NULL;
                    for (int cx = 0; cx < 2; ++cx) {
                        ptr = realPut((x * 2) + cx + 152, (y * 2) + cy + 8, 2, ptr);
                    }
                }
            } else {
                for (int cy = 0; cy < 2; ++cy) {
                    ptr = NULL;
                    for (int cx = 0; cx < 2; ++cx) {
                        ptr = realPut((x * 2) + cx + 152, (y * 2) + cy + 8, 0, ptr);
                    }
                }
            }

        }
    }
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
            updateDirection = 1;
            return 'q';
        case 28:
            updateDirection = 1;
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
        memcpy(line, buffer + (y * BUFFER_SIZEX), BUFFER_SIZEX);
    }

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

    uint8_t *ptr = &buffer[(_y0 * (BUFFER_SIZEX)) + (x0 / 4)]; /* skip to the line in pattern */
    uint8_t y;
    uint8_t pattern = (8 >> (x0 & 3));
    for (y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            *ptr |= pattern;
        }
        ptr += BUFFER_SIZEX;
    }

}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 4)]; /* skip to the line in pattern */
    }

    *ptr |= (8 >> (x & 3));

    return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {
    buffer[(y * (BUFFER_SIZEX)) + (x / 4)] |= (8 >> (x & 3));
}

void HUD_initialPaint() {
    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(19, 17 + i, menuItems[i]);
    }

    for (uint8_t y = 0; y < 200; ++y) {
        realPut(144, y, 1, NULL);
    }

    drawMap();

    for (uint16_t x = 0; x < 320; ++x) {
        realPut(x, 130, 1, NULL);
    }

    writeStrWithLimit(19, 14, "Direction: ", 31);
    updateDirection = 1;
    HUD_refresh();
}

void HUD_refresh() {

    for (uint8_t d = 0; d < 15; ++d) {
        writeStr(1 + d, 19, " ");
        writeStr(1 + d, 20, " ");
        writeStr(1 + d, 23, " ");
        writeStr(1 + d, 24, " ");
    }


    writeStrWithLimit(1, 18, "Object in room", 16);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(1, 19, "*", 16);
        }

        writeStrWithLimit(2, 19, item->name, 16);
    } else {
        writeStrWithLimit(2, 19, "Nothing", 16);
    }

    writeStrWithLimit(1, 21, "Object in hand", 16);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(1, 22, "*");
        }

        writeStrWithLimit(2, 22, item->name, 16);
    } else {
        writeStrWithLimit(2, 22, "Nothing", 16);
    }
}

