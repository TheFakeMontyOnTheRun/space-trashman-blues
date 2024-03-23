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
#include "Core.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "Renderer.h"
#include "SoundSystem.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"

extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;
extern struct Texture* textures;

void initHW(int argc, char** argv) {
    textBuffer = (char*)calloc(TEXT_BUFFER_SIZE, 1);
    messageLogBuffer = (char*)calloc(256, 1);
    collisionMap = (uint8_t*)calloc(256, 1);
    visMap = (enum EVisibility*)calloc(MAP_SIZE * MAP_SIZE, sizeof(enum EVisibility));
    distances = (struct Vec2i*)calloc(2 * MAP_SIZE * MAP_SIZE, sizeof(struct Vec2i));
    textures = (struct Texture*)calloc(TOTAL_TEXTURES, sizeof(struct Texture));
    itemsInMap = (uint8_t*)calloc(MAP_SIZE * MAP_SIZE, sizeof(uint8_t*));
    map = (uint8_t*)calloc(MAP_SIZE * MAP_SIZE, sizeof(uint8_t*));

    NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: @"base.pfs"];
    initFileReader([path UTF8String]);
    initZMap();
    graphicsInit();
    setupOPL2(1);
}

void shutdownHW(void);

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
    enableSmoothMovement = TRUE;
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

            
        case 53: //esc
            mBufferedCommand = kCommandBack;
            break;
            
            
        case 126:
            mBufferedCommand = kCommandUp;
            break;
        case 125:
            mBufferedCommand = kCommandDown;
            break;
            
        case 123:
            mBufferedCommand = kCommandLeft;
            if ((currentGameMenuState == kPlayGame ||
                currentGameMenuState == kBackToGame) &&
                currentPresentationState == kWaitingForInput
                ) {
                
                turnStep = 0;
                turnTarget = 200;
            }
            break;
            
        case 124:
            mBufferedCommand = kCommandRight;
            if ((currentGameMenuState == kPlayGame ||
                 currentGameMenuState == kBackToGame) &&
                currentPresentationState == kWaitingForInput
                ) {
                
                turnStep = 200;
                turnTarget = 0;
            }
            break;
        case -1:
            break;
        case 18:
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 19:
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
            
        case 20:
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 21:
//            renderingMethod = LUT;
            enableSmoothMovement = TRUE;
            needsToRedrawVisibleMeshes = TRUE;
            visibilityCached = FALSE;
            break;
        case 22:
            enableSmoothMovement = FALSE;
//            renderingMethod = FIXED;
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



