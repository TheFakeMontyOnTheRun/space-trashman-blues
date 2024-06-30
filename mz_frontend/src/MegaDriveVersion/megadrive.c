#include <genesis.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "GamepadUI.h"
#include "font.h"

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

extern uint8_t firstFrameOnCurrentState;

#define COOLDOWN_MAX 0x2EF

int16_t buffered = '.';
uint16_t cooldown;
uint16_t movementCooldown = 0;

void handleSystemEvents(void) {
    u16 value;
    buffered = '.';

    if (cooldown) {
        cooldown--;
    }

    if (movementCooldown) {
        movementCooldown--;
    }

    /* need to call it manually as we don't use SYS_doVBlankProcess() here */
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

        if ((value & BUTTON_A) && !cooldown) {
            cooldown = COOLDOWN_MAX;
            buffered = '1';
        }

        if ((value & BUTTON_B) && !cooldown) {
            cooldown = COOLDOWN_MAX;
            buffered = '2';
        }

        if ((value & BUTTON_C) && !cooldown) {
            cooldown = COOLDOWN_MAX;
            buffered = '3';
        }
    }
}

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    BMP_setPixelFast(x, y, colour);
}

void realPut(int x, int y, uint8_t colour) {
    BMP_setPixelFast(x, y, colour);
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour);
        }
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {

    if (y0 > y1) {
        int16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    colour += (colour << 4); /* double the pixel */
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

    colour += (colour << 4); /* double the pixel */

    for (int16_t x = _x0; x <= _x1; ++x) {
        BMP_setPixelFast(x, 16 + y, colour);
    }
}

static void joyEvent(u16 joy, u16 changed, u16 state) {

}

void clearGraphics(void) {
    BMP_clear();
}

void clearScreen(void) {
    BMP_clear();
}


void clearTextScreen(void) {
    for (int16_t c = 0; c < 23; ++c) {
        VDP_clearText(0, c, 256 / 8);
    }
    cooldown = COOLDOWN_MAX;
    movementCooldown = COOLDOWN_MAX;
}

void initHW(int argc, char** argv) {
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

    firstFrameOnCurrentState = 1;

    initGamepadUI();
}

void endFrame(void) {
    BMP_flip(1);
}

enum ECommand getInput(void) {

    switch(buffered) {
        case 'q':
            return kCommandLeft;
        case 'w':
            return kCommandUp;
        case 's':
            return kCommandDown;
        case 'e':
            return kCommandRight;
        case 'a':
            return kCommandStrafeLeft;
        case 'd':
            return kCommandStrafeRight;
        case 'l':
            return kCommandBack;

        case '1':
            if (waitForKey) {
                waitForKey = 0;
                firstFrameOnCurrentState = 1;
                needsToRedrawVisibleMeshes = 1;
                return kCommandNone;
            }

            return kCommandFire1;
        case '2':
            return kCommandFire2;
        case '3':
            return kCommandFire3;
        case '4':
            return kCommandFire4;
        case '5':
            return kCommandFire5;
        case '6':
            return kCommandFire6;
    }

    return kCommandNone;
}


void startFrame(int x, int y, int width, int height) {

}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) >> 1;
    int e2;
    for (;;) {

        if (x0 == x1 && y0 == y1) break;

        realPut(x0, y0, colour);

        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawTextAtWithMarginWithFiltering(const int _x, const int _y, int limitX, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {

    char textBuffer[2];
    char *charPtr = &textBuffer[0];

    int16_t len = strlen(text);
    int16_t x = _x;
    int16_t y = _y;
    textBuffer[1] = 0;
    char* str= text;

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
