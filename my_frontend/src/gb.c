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
    gotogxy(x, y);
    gprint(text);
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {

    if (y > YRES_FRAMEBUFFER ) {
        return;
    }

    if (x > XRES_FRAMEBUFFER ) {
        return;
    }


    plot(x, y, BLACK, SOLID);
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
    
    wait_vbl_done();
    int key = joypad();
    
    performAction();
    
    if(key & (J_UP|J_DOWN|J_LEFT|J_RIGHT|J_A | J_B)) {
        clearScreen();
        
        if(key & J_A) {
            return kCommandFire1;
        }

        if(key & J_B) {
            return kCommandFire2;
        }

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
    box( 0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, M_FILL);
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
    line(x0, y0, x1, y1);
}
