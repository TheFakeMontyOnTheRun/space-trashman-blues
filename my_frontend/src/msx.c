#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <msx.h>
#include <stdio.h>
#include <msx/gfx.h>

#include <games.h>
#include <psg.h>
#include <sound.h>
#include <stdlib.h>
#include <time.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

char getch(void);

extern const struct Pattern patterns[127];

extern uint8_t playerLocation;

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

uint8_t cursorPosition = 0;

uint8_t updateDirection;

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 35

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

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint16_t cooldown;

void init(void) {
    set_color(15, 1, 1);
    set_mode(mode_2);
    fill(MODE2_ATTR, 0xF1, MODE2_MAX);
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
    uint8_t chary = 0;
    uint8_t x = _x;

    for (; c < len && y < 64; ++c) {

        char cha = *ptr;

        if (x == limitX) {
            ++y;
            x = _x;
        } else if (cha == '\n') {
            ++y;
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

        uint8_t baseY = (y << 3);

        uint8_t *fontTop = &font[((cha - 32) << 3)];
        vwrite(fontTop, map_pixel(x << 3, y << 3), 8);

        ++x;
        ++ptr;
    }
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


    psg_init();
    psg_channels(chanAll, chanNone); /*  set all channels to tone generation */
    psg_volume(0, 10);
    psg_volume(1, 10);
    psg_volume(2, 10);

    psg_tone(0, psgT(262));

    psg_tone(0, 0);

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

void shutdownGraphics(void) {
}

void clearScreen(void) {
    set_mode(mode_2);
    fill(MODE2_ATTR, 0xF1, MODE2_MAX);
}

void clearGraphics(void) {
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void graphicsFlush(void) {
    uint8_t *ptr = &buffer[0];
    for (uint8_t y = 0; y < (BUFFER_RESY); y += 8) {
        /* 248 = ~7 */
        vwrite(ptr, ((y & 248) << 5), 16 * 8);
        ptr += 8 * 16;
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
    /* we cant afford to sleep */
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t _y0 = y0;
    uint8_t _y1 = y1;


    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    uint8_t patternLine = (_y0 & 7); /* which line inside the pattern; */
    uint8_t *ptr = &buffer[((_y0 & ~7) << 4) + (x0 & ~7) + (_y0 & 7)];

    uint8_t shiftXAnd7 = 128 >> (x0 & 7);

    for (uint8_t y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            *ptr |= shiftXAnd7;
        }

        ++patternLine;
        ++ptr;

        if (patternLine >= 8) {
            patternLine = 0;
            ptr += (16 * 8) - 8;
        }
    }
}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {
    if (ptr == NULL) {
        ptr = &buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)];
    }

    *ptr |= (128 >> (x & 7));

    return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {
    buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)] |= (128 >> (x & 7));
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
                        pset((x * 2) + 136 + cx, (y * 2) + cy + 8);
                    }
                }
            }
        }
    }
}

void HUD_initialPaint(void) {
    struct Room *room = getRoom(getPlayerRoom());

    draw(BUFFER_RESX, 0, BUFFER_RESX, 191);
    draw(0, 128, 255, 128);
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