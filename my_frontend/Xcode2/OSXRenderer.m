//
//  OSXRenderer.cpp
//  The Mistral Report
//
//  Created by Daniel Monteiro on 10/13/18.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Common.h"
#include "Enums.h"
#include "OSXRenderer.h"
#include "GameView.h"

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "KeyboardUI.h"
#include "font.h"


GameView* osxview;
uint8_t mPalette[256];
struct Bitmap *mFont;
uint8_t mBufferedCommand = '.';

void performAction();

void flushVirtualFramebuffer(void);

void graphicsInit() {
}

void graphicsShutdown() {
}

enum ECommand getInput(void) {
    int code = [ osxview getInput ];
    
    mBufferedCommand = kCommandNone;
    
    switch (code) {
        case 0: //a
        case 6: //z
        case 36: //enter
            mBufferedCommand = kCommandFire1;
            break;
            
        case 7: //x
            mBufferedCommand = kCommandFire2;
            break;
            
        case 8: //c
            mBufferedCommand = kCommandFire3;
            break;
            
        case 9: //v
            mBufferedCommand = kCommandFire4;
            break;
            
        case 1: //s
            mBufferedCommand = kCommandStrafeLeft;
            break;
        case 2: //d
            mBufferedCommand = kCommandStrafeRight;
            break;
            
#ifdef EMIT_QUIT_OPTION            
        case 53: //esc
            mBufferedCommand = kCommandQuit;
            break;
#endif
            
        case 126:
            mBufferedCommand = kCommandUp;
            break;
        case 125:
            mBufferedCommand = kCommandDown;
            break;
            
        case 123:
            mBufferedCommand = kCommandLeft;
            break;
            
        case 124:
            mBufferedCommand = kCommandRight;
            break;
        case -1:
            break;
        default:
            break;
    }
    
    performAction();
    
    return mBufferedCommand;
}

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
uint8_t updateDirection = 0;

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[128 * 128];
uint8_t vfb[256 * 192];

void graphicsPut(uint8_t x, uint8_t y) {
    framebuffer[(128 * y) + x] = 1;
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    int x;
    
    for (x = 0; x < XRES; ++x) {
        graphicsPut(x, *stencilPtr);
        ++stencilPtr;
    }
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;
    
    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }
    
    
    for (y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            graphicsPut(x0, y);
        }
    }
}

void clearGraphics(void) {
    memFill(framebuffer, 0, 128 * 128);
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

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    assert(y >= 0);
    assert(x >= 0);
    assert(x < 256);
    assert(y < 192);
    
    vfb[(256 * y) + x] = colour;
    
    return NULL;
}

void clearScreen(void) {
    fillRect(0, 0, 255, 192, 0, 0);
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {

    size_t len = strlen(text);
    int32_t dstX = x * 8;
    int32_t dstY = y * 8;

    size_t c;
    size_t d;
    uint8_t lastSpacePos = 0xFF;

    for (c = 0; c < len; ++c) {

        char currentChar = text[c];

        if (currentChar == '-') {
            currentChar = charToReplaceHifenWith;
        }

        if (currentChar == '\n' || dstX >= (margin)) {
            dstX = x * 8;
            dstY += 8;
            continue;
        }

        if (dstY >= YRES_FRAMEBUFFER) {
            return;
        }

        if (currentChar == ' ') {
            lastSpacePos = c;
        } else {
            if ((c - 1) == lastSpacePos) {
                d = c;
                while (d < len && text[d] != ' ') ++d;

                if ((dstX + ((d - c ) * 8)) >= margin ) {
                    dstX = x * 8;
                    dstY += 8;
                }
            }
        }


        if (currentChar >= 'a') {
            if (currentChar <= 'z') {
                currentChar = (currentChar - 'a') + 'A';
            } else {
                currentChar -= ('z' - 'a');
            }
        }

        uint8_t *fontTop = &font[((currentChar - 32) << 3)];

        for (int f = 0; f < 8; ++f) {
            int e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    realPut(dstX + (7 - e), dstY + (f), 1, NULL);
                } else {
                    realPut(dstX + (7 - e), dstY + (f), 0, NULL);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
        }
        dstX += 8;
    }
}


void flipRenderer(void) {
    int x, y;
    for (y = 0; y < 128; ++y) {
        for (x = 0; x < 128; ++x) {
            int index = framebuffer[(128 * y) + x];
            
            if (index < 0 || index >= 16) {
                continue;
            }
            
            realPut(x, y, index, NULL);
        }
    }
}

void graphicsFlush(void) {    
	if (needsToRedrawVisibleMeshes) {
        flipRenderer();
        clearGraphics();
	}
    flushVirtualFramebuffer();
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    uint8_t x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            if (!stipple || ((x + y) & 1 )) {
                realPut(x, y, colour, NULL);
            }
        }
    }
}


