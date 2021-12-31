#include <genesis.h>
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern int accessGrantedToSafe;

#define COOLDOWN_MAX 0x2EF

uint8_t buffered = '.';
int cursorPosition = 0;
uint16_t cooldown;

char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

void performAction() {
    struct Room *room = getRoom(getPlayerRoom());

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
            break;
        case 2:
            pickItem();
            break;
        case 3:
            dropItem();
            break;
        case 4:
            nextItemInHand();
            break;
        case 5:
            nextItemInRoom();
            break;
    }
}


static void handleInput()
{
    u16 value;
    buffered = '.';

    // need to call it manually as we don't use SYS_doVBlankProcess() here
    JOY_update();

    value = JOY_readJoypad(JOY_1);

    if (value & BUTTON_A)
    {
        if (value & BUTTON_LEFT) {
            buffered = 'a';
        } else if (value & BUTTON_RIGHT) {
            buffered = 'd';
        }
    } else {

        if (value & BUTTON_UP) {
            buffered = 'w';
        }

        if (value & BUTTON_DOWN) {
            buffered = 's';
        }

        if (value & BUTTON_LEFT) {
            buffered = 'q';
        }

        if (value & BUTTON_RIGHT) {
            buffered = 'e';
        }

        if (value & BUTTON_START) {
            buffered = 'k';
        }

        if ((value & BUTTON_B)&& !cooldown) {
            performAction();
            HUD_refresh();
            cooldown = COOLDOWN_MAX;
        }

        if ((value & BUTTON_C) && !cooldown) {
            cursorPosition = (cursorPosition + 1);

            if (cursorPosition >= 6) {
                cursorPosition = 0;
            }

            HUD_refresh();
            cooldown = COOLDOWN_MAX;
        }
    }
}

void shutdownGraphics() {
}




void writeStrWithLimit(int _x, int y, char *str, int limitX) {
    BMP_clearText(_x, y, strlen(str));
    BMP_drawText(str, _x, y);
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 31);
}

void graphicsPut( uint8_t x, uint8_t y) {
    BMP_setPixel(x, y, 0x11);
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t stipple = 1;

    if (y0 > y1) {
        int tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    uint8_t colour;

    if (shouldStipple < 4) {
        colour = shouldStipple;
        shouldStipple = 0;
    } else {
        colour = shouldStipple - 4;
        shouldStipple = 1;
    }

    for (int y = y0; y < y1; ++y ) {
        if (shouldStipple) {
            stipple = !stipple;
        }

        if (stipple) {
            BMP_setPixel(x0, y, 3 + colour);
        }
    }
}

static void joyEvent(u16 joy, u16 changed, u16 state) {

}

void showMessage(const char *message) {
    writeStr(1, 1, message, 2, 0);
}

void clearScreen() {
    BMP_waitWhileFlipRequestPending();
    BMP_clear();

    for ( int c = 0; c < 20; ++c) {
        BMP_clearText(0, c, 32);
    }
}

void clearGraphics() {
    BMP_waitWhileFlipRequestPending();
    BMP_clear();
}

void drawWindow(int tx, int ty, int tw, int th, const char* title ) {}

uint8_t getKey() {
    handleInput();
    return buffered;
}

void titleScreen() {
    int keepGoing = 1;
    clearGraphics();

    writeStr(1, 1, "Space Mare Imperium:", 2, 0);
    writeStr(1, 2, "     Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 7, "  Press start button ", 2, 0);

    while (keepGoing) {
        if (getKey() == 'k') {
            keepGoing = 0;
        }
    }
    clearScreen();
}

void puts(char *unused) {

}

void assert( int unused) {

}

void init() {
    char col;

    JOY_setEventHandler(joyEvent);
    VDP_setScreenWidth256();
    VDP_setHInterrupt(0);
    VDP_setHilightShadow(0);

    // reduce DMA buffer size to avoid running out of memory (we don't need it)
    DMA_setBufferSize(2048);

    // init Bitmap engine (require a ton shit of memory)
    BMP_init(TRUE, BG_A, PAL0, FALSE);


    col = 0xFF;
}

void graphicsFlush() {
    HUD_initialPaint();
    BMP_flip(1);
}

void HUD_initialPaint() {
    struct Room *room = getRoom(getPlayerRoom());


    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(16, 14 + i, i == cursorPosition ? ">" : " ", 2, 0);
        writeStr(17, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void HUD_refresh() {

    for (uint8_t i = 0; i < 6; ++i) {
        writeStr(16, 14 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
    }

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(16, 21, "*", 2, 0);
        }

        writeStrWithLimit(17, 21, item->name, 30);
    }

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStr(0, 1, "*", 2, 0);
        }

        writeStrWithLimit(1, 1, item->name, 14);
    }
}