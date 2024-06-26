//
//  GameView.m
//  Sub Mare Imperium: Derelict
//
//  Created by Daniel Monteiro on 2023-09-04.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import "GameView.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "Common.h"
#include "Enums.h"
#include "UI.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Core.h"
#include "Derelict.h"
#include "KeyboardUI.h"
#include "Renderer.h"

const char *mainText;

long timeUntilNextState;
enum EPresentationState currentPresentationState;
enum EGameMenuState currentGameMenuState;
int8_t cameraRotation = 0;
int8_t cursorPosition;
enum EGameMenuState nextNavigationSelection;
enum EGameMenuState menuStateToReturn;
enum ESoundDriver soundDriver = kNoSound;

extern GameView* osxview;

@implementation GameView

uint32_t palette[16];
uint32_t stretchedBuffer[ 256 * 192 ];
uint8_t framebuffer[128 * 128];
uint8_t vfb[256 * 192];

CGColorSpaceRef rgb;
CGDataProviderRef provider;
CGImageRef ref;
extern uint8_t mBufferedCommand;
float multiplier = 1.0f;
extern uint8_t updateDirection;
id delegate;


void graphicsFlush(void);

void stopSounds(void) {
    
}

void soundTick(void) {
    
}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

void endFrame(void) {
    if (needsToRedrawVisibleMeshes) {
        graphicsFlush();
    }
    flushVirtualFramebuffer();
}

void playSound(uint8_t unused) {
    
}

void handleSystemEvents(void) {
    
}

void initHW(int argc, char** argv) {
    initKeyboardUI();
    updateDirection = 0;
    
    mBufferedCommand = '.';
    memFill(framebuffer, 0, 128 * 128);
}


void shutdownGraphics(void) {
}

- (void) repaintGame:(NSTimer *)timer
{
    startFrame(0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER);
    menuTick(10);
    endFrame();

    
    [self setNeedsDisplay: YES ];
}

void setMultiplier(CGSize size) {
    
    if (((320.0f / 240.0f ) * size.height) < size.width ) {
        multiplier = (((float)size.height) / 240.0f);
    } else {
        multiplier = (((float)size.width) / 320.0f);
    }
}

- (id)initWithFrame:(NSRect)frame
{
    int r, g, b;
    
    self = [super initWithFrame:frame];
    if (self) {
        
        bufferedInput = -1;
        delegate = self;
        rgb = CGColorSpaceCreateDeviceRGB();
        
        
        osxview = self;
        palette[0] = 0xFF000099;
        palette[1] = 0xFFFFFFBF;
        palette[2] = 0xFFE0FFFF;
        palette[3] = 0xFFFF0000;
        palette[4] = 0xFFFFFFFF;
        palette[5] = 0xFF000000;
        palette[6] = 0xFF0000FF;
        palette[7] = 0xFFFF00FF;
        palette[8] = 0xFF00b7eb;
        palette[9] = 0xFFFFFF00;
        palette[10] = 0xFFAFEEEE;
        palette[11] = 0xFFffc0cb;
        palette[12] = 0xFF00FF00;
        palette[13] = 0xFFAAFFAA;
        palette[14] = 0xFF0000FF;
        palette[15] = 0xFFAAAAFF;
        
        
        initHW(0, NULL);
        enterState(kMainMenu);

        
        [self initTimer];
    }
    
    return self;
}

-(void)keyDown:(NSEvent *)theEvent
{
}

-(void)keyUp:(NSEvent*)event
{
    
    bufferedInput = [ event keyCode ];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

-(void) initTimer {
    [NSTimer scheduledTimerWithTimeInterval:0.05f
                                     target:self selector:@selector(repaintGame:) userInfo:nil repeats:YES];
    
}

- (int) getInput {
    int toReturn = bufferedInput;
    
    bufferedInput = -1;
    
    return toReturn;
}

-(void) stopTimer {
}

void shutdownHW(void) {
    CGColorSpaceRelease(rgb);
}

void flushVirtualFramebuffer() {
    int x,y;
    uint32_t pixel;
    uint32_t *bufferPtr = &stretchedBuffer[0];
    for ( y = 0; y < 192; ++y ) {
        for ( x = 0; x < 256; ++x ) {
            int index = vfb[(256 * y) + x];
            
            if (index < 0 || index >= 16) {
                continue;
            }
            
            pixel = palette[index];
            
            *bufferPtr = pixel;
            ++bufferPtr;
        }
    }    
}

- (void)drawRect:(NSRect)rect {

    CGRect bounds;
    //NSRectToCGRect is not available on 10.4
    bounds.origin.x = rect.origin.x;
    bounds.origin.y = rect.origin.y;
    bounds.size.width = rect.size.width;
    bounds.size.height = rect.size.height;
    
    float yMultiplier = ( (240.0f * multiplier) / 200.0f );
    
    setMultiplier(bounds.size);
    
    /*  MAC_OS_X_VERSION_10_10*/
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 101000
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
#else
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
    
    CGContextSaveGState(context);
    
    CGContextSetRGBFillColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(context, bounds);
    
    provider = CGDataProviderCreateWithData( NULL, &stretchedBuffer[0], 4 * XRES_FRAMEBUFFER * YRES_FRAMEBUFFER, NULL );
    ref = CGImageCreate( XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, 8, 32, 4 * XRES_FRAMEBUFFER, rgb, kCGBitmapByteOrder32Host | kCGImageAlphaNoneSkipLast, provider, NULL, 0, kCGRenderingIntentDefault );
    CGContextScaleCTM(context, multiplier, yMultiplier);
    CGContextDrawImage(context, CGRectMake( ((bounds.size.width / multiplier) - XRES_FRAMEBUFFER) / 2, ((bounds.size.height / yMultiplier) - YRES_FRAMEBUFFER) / 2, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER), ref);
    CGImageRelease(ref);
    CGDataProviderRelease(provider);
    
    CGContextRestoreGState(context);
}
@end
