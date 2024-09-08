#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <gb/gb.h>
#include <stdint.h>
#include <gb/drawing.h>

#include "Common.h"
#include "Enums.h"
#include "Core.h"
#include "Renderer.h"

#include "GamepadUI.h"
#include "UI.h"

extern uint8_t firstFrameOnCurrentState;
extern enum EGameMenuState currentGameMenuState;


extern enum EDirection playerDirection;
extern int8_t cameraX;
extern int8_t cameraZ;

void initHW(int argc, char **argv) {
    (void)argc;
    (void)argv;
    OBP1_REG = 0xE0U;
    initGamepadUI();
    needsToRedrawVisibleMeshes = 0;
    waitForKey = 0;
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {
   
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {

    if (y > YRES ) {
        return;
    }

    if (x > XRES ) {
        return;
    }


    plot(x, y, BLACK, SOLID);
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
    
    wait_vbl_done();
    int key = joypad();
    
    performAction();

    if(key & (J_UP|J_DOWN|J_LEFT|J_RIGHT)) {
        clearGraphics();
        if(key & J_UP) {
            return kCommandUp;
        }

        if(key & J_DOWN) {
            return kCommandDown;
        }

        if(key & J_LEFT) {
            return kCommandLeft;
        }

        if(key & J_RIGHT) {
            return kCommandRight;
        }
    }
    return kCommandNone;
}

void clearScreen(void) {
    box( 0, 0, XRES, YRES, M_FILL);
}

void clearGraphics(void) {
    box( 0, 0, XRES, YRES, M_FILL);
}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

void endFrame(void) {
    wait_vbl_done();
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    if (y1 > YRES ) {
        y1 = YRES;
    }

    if (y0 > YRES ) {
        y0 = YRES;
    }

    if (x0 > XRES ) {
        return;
    }

    line(x0, y0, x0, y1 );
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    int x;

    for (x = 0; x < XRES; ++x) {
        graphicsPut(x, *stencilPtr);
        ++stencilPtr;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    realPut(x, y, 1, NULL);
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
    int16_t dx = abs(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = abs(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) >> 1;
    int16_t e2;

    for (;;) {

        if (x0 == x1 && y0 == y1) return;

        realPut(x0, y0, colour, NULL);

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
