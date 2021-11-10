//
//  GameView.m
//  Madame Y
//
//  Created by Daniel Monteiro on 27/05/2020.
//  Copyright © 2020 Daniel Monteiro. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>

#import "GameView.h"

@implementation GameView


#define TRANSPARENCY_COLOR 17

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[160 * 200];

void tickRenderer();
void demoMain();
int32_t stretchedBuffer[ 160 * 200 ];
CGColorSpaceRef rgb;
CGDataProviderRef provider;
CGImageRef ref;
int initialized = 0;
    
- (void) repaintGame:(NSTimer *)timer
{

    [self setNeedsDisplay: YES ];
}


void initWindow(id view) {
    initialized = 1;
    
           
           CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
           rgb = CGColorSpaceCreateDeviceRGB();

    
           
           mBufferedCommand = '.';
           memset(framebuffer, 5, 160 * 200);

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
           
           
           [NSTimer scheduledTimerWithTimeInterval:0.01f
           target:view selector:@selector(repaintGame:) userInfo:nil repeats:YES];
    
           demoMain();
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
        if (!initialized) {
            initWindow(self);
        }
    }
    
    return self;
}

-(void)keyDown:(NSEvent *)theEvent
{
}

-(void)keyUp:(NSEvent*)event
{
    switch ([ event keyCode ]) {
        case 0: //a
        case 36: //enter
            break;
        case 1: //s
            mBufferedCommand = 's';
            break;
        case 53: //esc
            mBufferedCommand = 'q';
            break;
            
        case 6: //z
            mBufferedCommand = 'z';
            break;
        case 7: //x
            mBufferedCommand = 'x';
            break;
        case 126:
            mBufferedCommand = 'w';
            break;
        case 125:
            mBufferedCommand = 's';
            break;
        case 123:
            mBufferedCommand = 'a';
            break;
            
        case 124:
            mBufferedCommand = 'd';
            break;

    }
}

- (BOOL)acceptsFirstResponder {
    return YES;
}


-(void) stopTimer {
}


- (void)drawRect:(NSRect)rect {

    if (!initialized) {
        initWindow(self);
    }

    tickRenderer();
    
        for ( int y = 0; y < 200; ++y ) {
            for ( int x = 0; x < 160; ++x ) {
                
                uint8_t index = framebuffer[ (160 * ( y )) + (x)];
                uint32_t pixel = palette[ index ];
                uint32_t newPixel;
                
#ifdef __BIG_ENDIAN_
                newPixel = (((pixel & 0x000000FF) ) << 24) +
                ((((pixel & 0x0000FF00) >> 8) ) << 16) +
                ((((pixel & 0x00FF0000) >> 16) ) << 8 );
#else
                newPixel = (((pixel & 0x000000FF) ) << 8) +
                ((((pixel & 0x0000FF00) >> 8) ) << 16) +
                ((((pixel & 0x00FF0000) >> 16) ) << 24 );

#endif

            
                stretchedBuffer[ ( y * 160 ) + x ] = newPixel;
        
            }
        }
    
    
    
    
        
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_10
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
#else
    CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
#endif
    
    
    CGContextSaveGState(context);
    
    provider = CGDataProviderCreateWithData( NULL, &stretchedBuffer[0], 4 * 160 * 200, NULL );
    ref = CGImageCreate( 160, 200, 8, 32, 4 * 160, rgb, kCGBitmapByteOrder32Host, provider, NULL, 0, kCGRenderingIntentDefault );
    CGContextDrawImage(context, CGRectMake(0, 0, 160, 200), ref);
    CGImageRelease(ref);
    CGDataProviderRelease(provider);

    CGContextRestoreGState(context);
}
@end



void graphicsFlush();

void graphicsPut(uint8_t x, uint8_t y) {


    if (x < 0 || x > 127 || y < 0 || y > 127) {

    }

    framebuffer[(160 * y) + x] = 1;
}


void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    if (x0 == x1) {

        int16_t _y0 = y0;
        int16_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        for (int16_t y = _y0; y <= _y1; ++y) {
            graphicsPut(x0, y);
        }
        return;
    }

    if (y0 == y1) {
        int16_t _x0 = x0;
        int16_t _x1 = x1;

        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }

        for (int16_t x = _x0; x <= _x1; ++x) {
            graphicsPut(x, y0);
        }
        return;
    }

    //switching x0 with x1
    if (x0 > x1) {
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;

        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;
    }

    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

        int dx = abs(x1 - x0);
        int sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0);
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;  /* error value e_xy */

        while (1) {
            graphicsPut(x0, y0);
            /* loop */
            if (x0 == x1 && y0 == y1) return;
            int e2 = 2 * err;

            if (e2 >= dy) {
                err += dy; /* e_xy+e_x > 0 */
                x0 += sx;
            }

            if (e2 <= dx) {
                /* e_xy+e_y < 0 */
                err += dx;
                y0 += sy;
            }
        }
    }
}

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {
    fix_line(x0, y, x1, y);
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    fix_line(x0, y0, x0, y1);
}


void shutdownGraphics(void) {
    CGColorSpaceRelease(rgb);
}


void clearGraphics(void) {
    memset(framebuffer, 0, 160 * 200);
}


void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg) {

}

uint8_t getKey() {
    uint8_t old = mBufferedCommand;
    mBufferedCommand = '.';
    return old;
}

void init() {

}


void flipRenderer() {
  
}

void graphicsFlush() {
    flipRenderer();
}
