//
//  OSXRenderer.cpp
//  The Mistral Report
//
//  Created by Daniel Monteiro on 10/13/18.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Globals.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "SoundSystem.h"
#include "VisibilityStrategy.h"
#include "CPackedFileReader.h"

void initHW() {
    NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"base.pfs"];
    initFileReader([path UTF8String]);
    graphicsInit();
    setupOPL2();
}

int selected = 0;
extern int coords[6];

void shutdownHW();

#include "OSXRenderer.h"

#include "GameView.h"

GameView* osxview;
uint8_t mPalette[256];
struct Bitmap *mFont;

uint8_t getPaletteEntry(const uint32_t origin) {
    uint8_t shade;
    
    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }
    
    shade = 0;
    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;
    
    return shade;
}

void flipRenderer() {
    
}

void graphicsInit() {
    int r, g, b;
    
    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                mPalette[paletteEntry] = pixel;
            }
        }
    }
    
    [osxview setBuffer: framebuffer];
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
    mFont = loadBitmap("font.img");
    defaultFont = mFont;
}

void graphicsShutdown() {
}


void clear() {
}

void handleSystemEvents() {
    mBufferedCommand = kCommandNone;
    int code = [ osxview getInput ];
    
    switch (code) {
        case 0: //a
        case 36: //enter
            mBufferedCommand = kCommandFire4;
            break;
        case 9: //v
            mBufferedCommand = kCommandFire5;
            break;

        case 1: //s
            mBufferedCommand = kCommandStrafeLeft;
             coords[(selected * 2)] -= 2;
            break;
        case 2: //d
            mBufferedCommand = kCommandStrafeRight;
            coords[(selected * 2)] += 2;
            break;

        case 3: //f
            mBufferedCommand = kCommandFire6;
            break;
            
        case 53: //esc
            mBufferedCommand = kCommandBack;
            break;
            
        case 6: //z
            mBufferedCommand = kCommandFire1;
            break;
        case 7: //x
            mBufferedCommand = kCommandFire2;
            break;
        case 8: //c
            mBufferedCommand = kCommandFire3;
            break;
            
        case 126:
            mBufferedCommand = kCommandUp;
            if ( renderingMethod == FIXED )
            {
            coords[(selected * 2) + 1] -= 2;
            }
            break;
        case 125:
            mBufferedCommand = kCommandDown;
            if ( renderingMethod == FIXED )
            {
            coords[(selected * 2) + 1] += 2;
            }
            break;
            
        case 123:
            mBufferedCommand = kCommandLeft;
            turnStep = 0;
            turnTarget = 256;
            break;
            
        case 124:
            mBufferedCommand = kCommandRight;
            turnStep = 256;
            turnTarget = 0;
            break;
        case -1:
            break;
        case 18:
            selected = 0;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 19:
            selected = 1;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
            
        case 20:
            selected = 2;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 21:
            renderingMethod = LUT;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 22:
            renderingMethod = FIXED;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;

        default:
            break;
    }
    
    
    if (mBufferedCommand != kCommandLeft && mBufferedCommand != kCommandRight && mBufferedCommand != kCommandNone) {
        turnStep = 0;
        turnTarget = 0;
    }
    
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
}



