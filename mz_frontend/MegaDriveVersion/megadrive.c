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


void refreshJustGraphics(void) {
    renderScene();
    graphicsFlush();
}

void backToGraphics(void) {
    clearScreen();
    HUD_initialPaint();
    refreshJustGraphics();
}

void performAction(void) {

}


static void handleInput(void) {
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

void shutdownGraphics(void) {
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
        BMP_setPixelFast(x, 16 + y, colour);
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {


    if (y0 > y1) {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    colour += (colour << 4); //double the pixel
    for (int16_t y = y0; y < y1; ++y) {
        BMP_setPixelFast(x0, 16 + y, colour);
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

    colour += (colour << 4); //double the pixel

    for (int16_t x = _x0; x <= _x1; ++x) {
        BMP_setPixelFast(x, 16 + y, colour);
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

void clearScreen(void) {
}

void clearGraphics(void) {
    BMP_clear();
}

void clearTextScreen(void) {
    for (int16_t c = 0; c < 23; ++c) {
        VDP_clearText(0, c, 256 / 8);
    }
    cooldown = COOLDOWN_MAX;
    movementCooldown = COOLDOWN_MAX;
}

void enterTextMode(void) {
    clearGraphics();
    BMP_flip(1);
    clearTextScreen();
}

void exitTextMode(void) {
    clearTextScreen();
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

uint8_t getKey(void) {
    handleInput();
    return buffered;
}

void sleepForMS(uint32_t ms) {
    //we cant afford to sleep
}

void titleScreen(void) {

}

void puts(char *unused) {

}

void assert(int unused) {

}

void init(void) {
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

void graphicsFlush(void) {
    BMP_flip(1);
}

void HUD_initialPaint(void) {
}

void HUD_refresh(void) {

}
