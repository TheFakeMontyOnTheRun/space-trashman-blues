#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <games.h>
#include <graphics.h>

#include "Enums.h"
#include "Core.h"
#include "Renderer.h"

#include "KeyboardUI.h"
#include "UI.h"
#include "font.h"

uint8_t updateDirection;

extern uint8_t firstFrameOnCurrentState;

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

void initHW(int argc, char **argv) {
    (void)argc;
    (void)argv;
    initKeyboardUI();
    clg();
    updateDirection = 1;
    needsToRedrawVisibleMeshes = 0;
}

void writeStrWithLimit(uint8_t _x, uint8_t y, const char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {
    (void)fg;
    (void)bg;
    uint8_t len = strlen(text);
    const char *ptr = text;
    uint8_t c = 0;
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

        uint8_t *fontTop = &font[((cha - 32) << 3)];


        for (uint8_t d = 0; d < 8; ++d) {
            uint8_t e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    plot((x * 8) + (7 - e), (y * 8) + d);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
        }

        ++x;
        ++ptr;
    }
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    if (colour) {
        plot(x, y);
    } else {
        unplot(x, y);
    }

    return NULL;
}

void handleSystemEvents(void) {}

uint8_t inputCounter = 0;
enum ECommand getInput(void) {
    performAction();

    int j = joystick(4);
    if (j & MOVE_RIGHT) {
        clearGraphics();
        return kCommandRight;
    }
    if (j & MOVE_LEFT) {
        clearGraphics();
        return kCommandLeft;
    }
    if (j & MOVE_DOWN) {
        clearGraphics();
        return kCommandDown;
    }
    if (j & MOVE_UP) {
        clearGraphics();
        return kCommandUp;
    }

    if (j & MOVE_FIRE1) {
        clearGraphics();
        return kCommandFire1;
    }

    if (j & MOVE_FIRE2) {
        clearGraphics();
        return kCommandFire2;
    }

    if (j & MOVE_FIRE3) {
        clearGraphics();
        return kCommandFire3;
    }


    if (j & MOVE_FIRE4) {
        clearGraphics();
        return kCommandFire4;
    }

    return kCommandNone;

}

void clearScreen(void) {
    clg();
}

void clearGraphics(void) {
    clga(0, 0, 128, 128);
}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

void endFrame(void) {

}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
    draw(x0, y0, x0, y1);
}

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    (void)colour;
    realPut(x, y, colour, ptr);
    return NULL;
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    uint16_t x;

    for (x = 0; x < XRESMINUSONE; ) {
        uint8_t y = *stencilPtr;

        plot(x, y);

        ++x;
        ++stencilPtr;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    plot(x, y);
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    uint8_t y;
    uint16_t x;

    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            if (!stipple || ((x + y) & 1 )) {
                realPut(x, y, colour, NULL);
            }
        }
    }
}


void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    if (colour) {
        draw(x0, y0, x1, y1);
    } else {
        undraw(x0, y0, x1, y1);
    }
}
