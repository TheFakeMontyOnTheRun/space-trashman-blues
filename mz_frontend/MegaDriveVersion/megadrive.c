#include <genesis.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "Engine.h"

void graphicsFlush(void);

void nextItemInHand(void);

void useItemInHand(void);

void nextItemInRoom(void);

void interactWithItemInRoom(void);

void pickOrDrop(void);

void dropItem(void);

void pickItem(void);

void clearGraphics(void);

void renderScene(void);

void enterTextMode(void);

void exitTextMode(void);


extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

#define COOLDOWN_MAX 0x2EF

int16_t buffered = '.';
uint16_t cooldown;
uint16_t movementCooldown = 0;

char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

void refreshJustGraphics() {
    renderScene();
    graphicsFlush();
}

void backToGraphics() {
    clearScreen();
    HUD_initialPaint();
    refreshJustGraphics();
}

void performAction() {

    switch (getGameStatus()) {
        case kBadVictory:
            showMessage("Victory! Too bad you didn't survive\nto tell the story\n\n\n\n\n\n");
            while (1);

        case kBadGameOver:
            showMessage("You're dead! And so are millions of\n"
                        "other people on the path of\n"
                        "destruction faulty reactor\n\n\n\n\n\n");
            while (1);

        case kGoodVictory:
            showMessage("Victory! You managed to destroy the\nship and get out alive\n\n\n\n\n\n");
            while (1);

        case kGoodGameOver:
            showMessage("You failed! While you fled the ship\n"
                        "alive, you failed to prevent the \n"
                        "worstscenario and now EVERYBODY is\n"
                        "dead (and that includes you!)\n\n\n\n\n");
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


static void handleInput() {
    u16 value;
    buffered = '.';

    if (cooldown) {
        cooldown--;
    }

    if (movementCooldown) {
        movementCooldown--;
    }

    // need to call it manually as we don't use SYS_doVBlankProcess() here
    JOY_update();

    value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_A) {
        if (value & BUTTON_LEFT) {
            buffered = 'a';
        } else if (value & BUTTON_RIGHT) {
            buffered = 'd';
        }
    } else {

        if (value & BUTTON_UP && !movementCooldown) {
            buffered = 'w';
        }

        if (value & BUTTON_DOWN && !movementCooldown) {
            buffered = 's';
        }

        if (value & BUTTON_LEFT && !movementCooldown) {
            buffered = 'q';
        }

        if (value & BUTTON_RIGHT && !movementCooldown) {
            buffered = 'e';
        }

        if (value & BUTTON_START) {
            buffered = 'k';
        }

        if ((value & BUTTON_B) && !cooldown) {
            performAction();
            HUD_refresh();
            cooldown = COOLDOWN_MAX;
            buffered = 'p';
        }

        if ((value & BUTTON_C) && !cooldown) {
            cursorPosition = (cursorPosition + 1);

            if (cursorPosition >= 6) {
                cursorPosition = 0;
            }

            HUD_refresh();
            cooldown = COOLDOWN_MAX;
            buffered = 'p';
        }
    }
}

void shutdownGraphics() {
}


void writeStrWithLimit(int16_t _x, int16_t y, char *str, int16_t limitX) {

    char textBuffer[2];
    char *charPtr = &textBuffer[0];

    int16_t len = strlen(str);
    int16_t x = _x;
    textBuffer[1] = 0;

    for (int16_t c = 0; c < len && y < 19; ++c) {

        char cha = *str;

        if (x == limitX) {
            ++y;
            x = _x;
        } else if (cha == '\n') {
            ++y;
            x = _x;
            ++str;
            continue;
        }

        *charPtr = ' ';
        VDP_drawText(charPtr, x, y);
        *charPtr = cha;
        VDP_drawText(charPtr, x, y);
        ++x;
        ++str;
    }
}

void writeStr(int16_t _x, int16_t y, const char *text, uint16_t fg, uint16_t bg) {
    writeStrWithLimit(_x, y, text, 31);
}

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    if (colour >= 16) {
        if ((x + y) & 1) {
            BMP_setPixelFast(x, 16 + y, 0);
        } else {
            BMP_setPixelFast(x, 16 + y, colour - 16);
        }
    } else {
        BMP_setPixelFast(x, 16 + y, colour);
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {


    if (y0 > y1) {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    int16_t stipple;

    if (colour < 16) {
        colour += (colour << 4); //double the pixel
        for (int16_t y = y0; y < y1; ++y) {
            BMP_setPixelFast(x0, 16 + y, colour);
        }
    } else {
        stipple = (x0 & 1);
        colour -= 16;
        colour += (colour << 4); //double the pixel

        for (int16_t y = y0; y < y1; ++y) {
            stipple = ~stipple;

            if (stipple) {
                BMP_setPixelFast(x0, 16 + y, colour);
            } else {
                BMP_setPixelFast(x0, 16 + y, 0);
            }
        }
    }
}


void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0) {
        return;
    }

    int16_t _x0 = x0;
    int16_t _x1 = x1;

    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }

    if (_x0 < 0) {
        _x0 = 0;
    }

    if (_x1 >= 128) {
        _x1 = 127;
    }

    if (colour < 16) {
        colour += (colour << 4); //double the pixel

        for (int16_t x = _x0; x <= _x1; ++x) {
            BMP_setPixelFast(x, 16 + y, colour);
        }
    } else {
        colour -= 16;
        colour += (colour << 4); //double the pixel
        int16_t stipple = ((x0 + y) & 1);

        for (int16_t x = _x0; x <= _x1; ++x) {
            stipple = ~stipple;

            if (stipple) {
                BMP_setPixelFast(x, 16 + y, colour);
            } else {
                BMP_setPixelFast(x, 16 + y, 0);
            }
        }
    }
}

static void joyEvent(u16 joy, u16 changed, u16 state) {

}

void showMessage(const char *message) {
    enterTextMode();
    int16_t keepGoing = 1;

    for (int16_t i = 0; i < 19; ++i) {
        VDP_clearText(16, i, 16);
    }

    writeStrWithLimit(1, 5, message, 31);
    writeStrWithLimit(6, 17, "Press Start to continue", 31);

    while (keepGoing) {
        if (getKey() == 'k') {
            keepGoing = 0;
        }
    }

    clearScreen();

    for (int16_t i = 0; i < 19; ++i) {
        VDP_clearText(16, i, 16);
    }

    exitTextMode();
    backToGraphics();
}

void clearScreen() {
}

void clearGraphics() {
    BMP_clear();
}

void clearTextScreen() {
    for (int16_t c = 0; c < 23; ++c) {
        VDP_clearText(0, c, 256 / 8);
    }
    cooldown = COOLDOWN_MAX;
    movementCooldown = COOLDOWN_MAX;
}

void enterTextMode() {
    clearGraphics();
    BMP_flip(1);
    clearTextScreen();
}

void exitTextMode() {
    clearTextScreen();
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

uint8_t getKey() {
    handleInput();
    return buffered;
}

void sleepForMS(uint32_t ms) {
    //we cant afford to sleep
}

void titleScreen() {
    int16_t keepGoing = 1;
    clearGraphics();
    writeStr(1, 5, "   Sub Mare Imperium  ", 2, 0);
    writeStr(1, 6, "        Derelict        ", 2, 0);
    writeStr(1, 8, "   by Daniel Monteiro   ", 2, 0);
    writeStr(1, 10, "   Press start button!  ", 2, 0);

    while (keepGoing) {
        if (getKey() == 'k') {
            keepGoing = 0;
        }
    }


    VDP_clearText(1, 5, 24);
    VDP_clearText(1, 6, 24);
    VDP_clearText(1, 8, 24);
    VDP_clearText(1, 10, 24);

    clearScreen();
}

void puts(char *unused) {

}

void assert(int unused) {

}

void init() {
    JOY_setEventHandler(joyEvent);
    VDP_setScreenWidth256();
    VDP_setHInterrupt(0);
    VDP_setHilightShadow(0);

    DMA_setBufferSize(2048);

    /* create virtual 256x160 framebuffer */
    BMP_init(TRUE, BG_B, PAL0, 1);

    PAL_setColor(0, RGB24_TO_VDPCOLOR(0x000000));
    PAL_setColor(1, RGB24_TO_VDPCOLOR(0x0000AA));
    PAL_setColor(2, RGB24_TO_VDPCOLOR(0x00AA00));
    PAL_setColor(3, RGB24_TO_VDPCOLOR(0x00AAAA));
    PAL_setColor(4, RGB24_TO_VDPCOLOR(0xAA0000));
    PAL_setColor(5, RGB24_TO_VDPCOLOR(0xAA00AA));
    PAL_setColor(6, RGB24_TO_VDPCOLOR(0xAA5500));
    PAL_setColor(7, RGB24_TO_VDPCOLOR(0xAAAAAA));
    PAL_setColor(8, RGB24_TO_VDPCOLOR(0x555555));
    PAL_setColor(9, RGB24_TO_VDPCOLOR(0x5555FF));
    PAL_setColor(10, RGB24_TO_VDPCOLOR(0x55FF55));
    PAL_setColor(11, RGB24_TO_VDPCOLOR(0x55FFFF));
    PAL_setColor(12, RGB24_TO_VDPCOLOR(0xFF5555));
    PAL_setColor(13, RGB24_TO_VDPCOLOR(0xFF55FF));
    PAL_setColor(14, RGB24_TO_VDPCOLOR(0xFFFF55));
    PAL_setColor(15, RGB24_TO_VDPCOLOR(0xFFFFFF));
}

void graphicsFlush() {
    BMP_flip(1);
}

void HUD_initialPaint() {
    for (int16_t i = 0; i < 6; ++i) {
        writeStr(16, 13 + i, i == cursorPosition ? ">" : " ", 2, 0);
        writeStr(17, 13 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void HUD_refresh() {

    for (int16_t i = 0; i < 13; ++i) {
        VDP_clearText(16, i, 16);
    }

    for (int16_t i = 0; i < 6; ++i) {
        writeStr(16, 13 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
    }


    writeStrWithLimit(16, 5, "Object in hand:", 31);
    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(16, 6, "*", 2, 0);
        }

        writeStrWithLimit(17, 6, item->name, 31);
    } else {
        writeStrWithLimit(16, 6, "Nothing", 31);
    }

    writeStrWithLimit(16, 8, "Object in room:", 31);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(16, 9, "*", 31);
        }

        writeStrWithLimit(17, 9, item->name, 31);
    } else {
        writeStrWithLimit(16, 9, "Nothing", 31);
    }
}
