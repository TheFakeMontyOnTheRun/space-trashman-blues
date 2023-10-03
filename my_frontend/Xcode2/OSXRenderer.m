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

#include "Enums.h"
#include "OSXRenderer.h"
#include "GameView.h"

#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "KeyboardUI.h"
#include "Menu.h"
#include "font.h"


GameView* osxview;
uint8_t mPalette[256];
struct Bitmap *mFont;
uint8_t mBufferedCommand = '.';

void flushVirtualFramebuffer(void);

uint8_t getPaletteEntry(const uint32_t origin) {
    uint8_t shade;
    /*
    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }
    */
    shade = 0;
    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;
    
    return shade;
}

void graphicsInit() {
}

void graphicsShutdown() {
}

enum ECommand getInput(void) {
    int code = [ osxview getInput ];
    
    mBufferedCommand = '.';
    
    switch (code) {
        case 0: //a
        case 6: //z
        case 36: //enter
            mBufferedCommand = 'z';
            break;
            
        case 7: //x
            mBufferedCommand = 'x';
            break;
            
        case 8: //c
            mBufferedCommand = 'c';
            break;
            
        case 9: //v
            mBufferedCommand = 'v';
            break;
            
        case 1: //s
            mBufferedCommand = 's';
            break;
        case 2: //d
            mBufferedCommand = 'd';
            break;
            
            
        case 53: //esc
            mBufferedCommand = 'l';
            break;
            
            
        case 126:
            mBufferedCommand = 'w';
            break;
        case 125:
            mBufferedCommand = 's';
            break;
            
        case 123:
            mBufferedCommand = 'q';
            break;
            
        case 124:
            mBufferedCommand = 'e';
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
uint8_t updateDirection;

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

void clearTextScreen(void) {
    fillRect(0, 129, 256, 192, 0);
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
    memset(framebuffer, 0, 128 * 128);
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
    vfb[(256 * y) + x] = colour;
    
    return NULL;
}

void clearScreen(void) {
    fillRect(0, 0, 256, 192, 0);
}

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {
    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t x = _x;
	int d;
    
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
        
        
        for (d = 0; d < 8; ++d) {
            int e;
            uint8_t chunk = *fontTop;
            
            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    realPut(8 * x + (7 - e), 8 * y + (d), 1, NULL);
                } else {
                    realPut(8 * x + (7 - e), 8 * y + (d), 0, NULL);
                }
                chunk = chunk >> 1;
            }
            
            
            fontTop++;
        }
        
        ++x;
        ++ptr;
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
    if (updateDirection) {
        updateDirection = 0;
        switch (getPlayerDirection()) {
            case 0:
                writeStrWithLimit(12, 17, "N", 31, 2, 0);
                break;
            case 1:
                writeStrWithLimit(12, 17, "E", 31, 2, 0);
                break;
            case 2:
                writeStrWithLimit(12, 17, "S", 31, 2, 0);
                break;
            case 3:
                writeStrWithLimit(12, 17, "W", 31, 2, 0);
                break;
        }
    }
    
    flipRenderer();
    clearGraphics();
    flushVirtualFramebuffer();
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour, NULL);
        }
    }
}


