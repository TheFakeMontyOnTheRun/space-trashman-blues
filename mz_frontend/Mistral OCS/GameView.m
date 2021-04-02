//
//  GameView.m
//  The Mistral Report
//
//  Created by Daniel Monteiro on 10/13/18.
//  Copyright (c) 2018 Daniel Monteiro. All rights reserved.
//

#import "GameView.h"

#include "Enums.h"
#include "Common.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"

extern         GameView* osxview;

void initHW();

void shutdownHW();
void enterState(enum EGameMenuState newState);

@implementation GameView

uint32_t stretchedBuffer[ 320 * 200 ];
CGColorSpaceRef rgb;
CGDataProviderRef provider;
CGImageRef ref;
NSMutableSet* playingSounds;

int nextAudioChannel = -1;
float multiplier = 1.0f;

//NSString* fileNamesForSounds[] = {@"menu_move", @"menu_select", @"gotclue", @"detected2", @"bong", @"fire", @"enemyfire", @"detected2" };

NSSound* playerSounds[8];

id delegate;

void setupOPL2(int port) {
    /*
    NSBundle *bundle = [NSBundle mainBundle];
	playingSounds = [[NSMutableSet alloc] init];
	
    for ( int c = 0; c < 8; ++c ) {
		NSString* path = [[NSURL fileURLWithPath: [ bundle pathForResource: fileNamesForSounds[c] ofType:@"wav"]] path ];
        playerSounds[c] = [[NSSound alloc] initWithContentsOfFile: path byReference: NO];
		[playerSounds[c] setDelegate: delegate ];
    }
     */
}


void stopSounds() {}

void playSound( const int action ){
    /*
	NSSound *original = playerSounds[action];
	NSSound *newSound =  (NSSound*)[original copy ];
	[playingSounds addObject: newSound ];
	[newSound play];
     */
}

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool {
    /*
	[sound release];
	[playingSounds removeObject: sound ];
     */
}

void soundTick() {}

void muteSound() {}


- (void) repaintGame:(NSTimer *)timer
{
    isRunning = menuTick(50);
    
    if (!isRunning) {
        exit(0);
    }

    [self setNeedsDisplay: YES ];
}

void startup() {}

unsigned long getMilliseconds() {
    return 0;
}

void graphicsPut(int x, int y, uint8_t pixel ) {
    int offset = (320 * y) + (x << 1);
    uint8_t *ptr = &framebuffer[offset];
    *ptr = pixel;
    ++ptr;
    *ptr = pixel;
}

void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t pixel ) {
    
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
        int8_t sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;  /* error value e_xy */
        uint8_t offset;
        while (1) {
            
            offset = (320 * y0) + (2 * x0);
            framebuffer[offset] = pixel;
            framebuffer[offset + 1] = pixel;
            
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

void graphicsHorizontalLine(int16_t x0, int16_t x1, int16_t y, uint8_t pixel) {
    int offset;
    uint8_t *ptr;
    int16_t _x0 = x0;
    int16_t _x1 = x1;
    
    
    
    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }
    
    if (_x0 >= 128 || _x1 < 0 ) {
        return;
    }
    
    if ( _x0 < 0 ) {
        _x0 = 0;
    }
    
    if ( _x1 >= 128 ) {
        _x0 = 127;
    }
    
    offset = (320 * y) + (_x0 << 1);
    ptr = &framebuffer[offset];
    
    memset( ptr, pixel, (_x1 - _x0 ) * 2 );
}

void graphicsVerticalLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel ) {
    int offset;
    uint8_t *ptr;
    int16_t _y0 = y0;
    int16_t _y1 = y1;
    
    if (x0 < 0 || x0 >= 256 ) {
        return;
    }
    
    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }
    
    if ( _y0 >= 128 || _y1 < 0 ) {
        return;
    }
    
    if ( _y0 < 0) {
        _y0 = 0;
    }
    
    if ( _y1 >= 128) {
        _y1 = 127;
    }
    
    offset = (320 * _y0) + (x0 << 1);
    ptr = &framebuffer[offset];
    
    for (int16_t y = _y0; y <= _y1; ++y) {
        *ptr = pixel;
        ++ptr;
        *ptr = pixel;
        ptr += 319;
    }
}



void setMultiplier(NSSize size) {
	
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
        buffer = NULL;
		delegate = self;
		rgb = CGColorSpaceCreateDeviceRGB();

        for (r = 0; r < 256; r += 16) {
            for (g = 0; g < 256; g += 8) {
                for (b = 0; b < 256; b += 8) {
                    uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                    uint8_t paletteEntry = getPaletteEntry(pixel);
                    uint32_t newPixel;
           
                    
                    #ifndef __BIG_ENDIAN_
                                newPixel = (((pixel & 0x000000FF) - 0x38) << 24) +
                                    ((((pixel & 0x0000FF00) >> 8) - 0x18) << 16) +
                                    ((((pixel & 0x00FF0000) >> 16) - 0x10) << 8 );
                    #else
                                newPixel = (((pixel & 0x000000FF) - 0x38 ) << 8) +
                                    ((((pixel & 0x0000FF00) >> 8) - 0x18) << 16) +
                                    ((((pixel & 0x00FF0000) >> 16) -0x10) << 24 );

                    #endif
                    
                    palette[paletteEntry] = newPixel;
                }
            }
        }
        
        osxview = self;
        
        srand(time(NULL));
        
        initHW();
        
        enterState(kMainMenu);
        
        menuTick(50);
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

- (void)setBuffer:(uint8_t*) bufferFromOutside {
    buffer = bufferFromOutside;
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

void shutdownHW() {
	CGColorSpaceRelease(rgb);
}


- (void)drawRect:(NSRect)rect {

    NSRect bounds = self.bounds;
    float yMultiplier = ( (240.0f * multiplier) / 200.0f );
    
	if (buffer == NULL ){
        return;
    }
    
    setMultiplier(bounds.size);
    
    for ( int y = 0; y < 200; ++y ) {
        
        if ( y < dirtyLineY0 || y > dirtyLineY1 ) {
            continue;
        }
        
        for ( int x = 0; x < 320; ++x ) {
            uint32_t offset = ( 320 * y ) + x;
            uint8_t index = buffer[ offset ];
            uint32_t pixel = palette[ index ];
            stretchedBuffer[offset] = pixel;
        }
    }
                                        /*  MAC_OS_X_VERSION_10_10*/
    #if MAC_OS_X_VERSION_MIN_REQUIRED >= 101000
        CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    #else
        CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] graphicsPort];
    #endif
    
	CGContextSaveGState(context);
    
    CGContextSetRGBFillColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(context, NSRectToCGRect(bounds));
	
	provider = CGDataProviderCreateWithData( NULL, &stretchedBuffer[0], 4 * 320 * 200, NULL );
	ref = CGImageCreate( 320, 200, 8, 32, 4 * 320, rgb, kCGBitmapByteOrder32Host, provider, NULL, 0, kCGRenderingIntentDefault );
    CGContextScaleCTM(context, multiplier, yMultiplier);
	CGContextDrawImage(context, CGRectMake( ((bounds.size.width / multiplier) - 320) / 2, ((bounds.size.height / yMultiplier) - 200) / 2, 320, 200), ref);
	CGImageRelease(ref);
	CGDataProviderRelease(provider);

	CGContextRestoreGState(context);
}
@end
