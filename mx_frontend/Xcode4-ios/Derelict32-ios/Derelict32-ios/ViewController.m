//
//  ViewController.m
//  Derelict32-ios
//
//  Created by Daniel Monteiro on 3/11/24.
//  Copyright (c) 2024 Daniel Monteiro. All rights reserved.
//

#import "ViewController.h"


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

int bufferedInput;
struct Bitmap *mFont;
char *textBuffer;
enum ESoundDriver soundDriver;
float multiplier = 1.0f;
uint32_t stretchedBuffer[ XRES_FRAMEBUFFER * YRES_FRAMEBUFFER ];
uint8_t flippedBuffer[XRES_FRAMEBUFFER*YRES_FRAMEBUFFER];
extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;
extern struct Texture* textures;

CGColorSpaceRef rgb;
CGDataProviderRef provider;
CGImageRef ref;


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

void flipRenderer() {
    
}

void graphicsInit() {
    
}

void graphicsShutdown() {
}


void clear() {
}

void handleSystemEvents() {
    
}



void setupOPL2(int port) {
}


void stopSounds() {
}

void playSound( const int action ){
}


void soundTick() {}

void muteSound() {}


@interface ViewController ()

@end

@implementation ViewController


- (void) repaintGame:(NSTimer *)timer
{
    isRunning = menuTick(50);
    
    if (!isRunning) {
        exit(0);
    }
    
    [self draw];
    
    [playField setNeedsDisplay ];
}

void setMultiplier(CGSize size) {
    
    if (((320.0f / 240.0f ) * size.height) < size.width ) {
        multiplier = (((float)size.height) / 240.0f);
    } else {
        multiplier = (((float)size.width) / 320.0f);
    }
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
    free(textBuffer);
    free(messageLogBuffer);
    free(collisionMap);
    free(visMap);
    free(distances);
    free(textures);
    free(itemsInMap);
    free(map);
    CGColorSpaceRelease(rgb);
}


- ( void ) draw {
    UIGraphicsBeginImageContext( playField.frame.size);
    

    
    int x,y;
    CGRect bounds;
    //NSRectToCGRect is not available on 10.4
    bounds.origin.x =     playField.frame.origin.x;
    bounds.origin.y =     playField.frame.origin.y;
    bounds.size.width =     playField.frame.size.width;
    bounds.size.height =     playField.frame.size.height;
    
    float yMultiplier = -( (240.0f * multiplier) / 200.0f );
    
   
    setMultiplier(bounds.size);
	
	
	renderPageFlip(&flippedBuffer[0], &framebuffer[0], &previousFrame[0], turnStep, turnTarget, 0);

    uint8_t *pixelPtr = &flippedBuffer[0];
    uint32_t *bufferPtr = &stretchedBuffer[0];

    for ( y = 0; y < YRES_FRAMEBUFFER; ++y ) {
		for ( x = 0; x < XRES_FRAMEBUFFER; ++x ) {
			uint8_t index = *pixelPtr;
            uint32_t pixel = palette[ index ];
            *bufferPtr = pixel;
            ++pixelPtr;
            ++bufferPtr;
        }
    }
    
    memcpy( previousFrame, framebuffer, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER);

    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSaveGState(context);
    
    CGContextSetRGBFillColor(context, 0.0f, 0.0f, 0.0f, 1.0f);
    CGContextFillRect(context, bounds);
    
    provider = CGDataProviderCreateWithData( NULL, &stretchedBuffer[0], 4 * XRES_FRAMEBUFFER * YRES_FRAMEBUFFER, NULL );
    ref = CGImageCreate( XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, 8, 32, 4 * XRES_FRAMEBUFFER, rgb, kCGBitmapByteOrder32Host | kCGImageAlphaNoneSkipLast, provider, NULL, 0, kCGRenderingIntentDefault );
    CGContextScaleCTM(context, multiplier, yMultiplier);
    CGContextDrawImage(context, CGRectMake( ((bounds.size.width / multiplier) - XRES_FRAMEBUFFER) / 2, ((bounds.size.height / yMultiplier) - YRES_FRAMEBUFFER) / 2, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER), ref);
    CGImageRelease(ref);
    CGDataProviderRelease(provider);
    
    
    playField.image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
}

- (void)viewDidLoad
{
    int r, g, b;
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
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
    
    
    initHW(0, NULL);
    
    needsToRedrawVisibleMeshes = TRUE;
    visibilityCached = FALSE;
    enableSmoothMovement = TRUE;
    mFont = loadBitmap("font.img");
    defaultFont = mFont;

    enterState(kPlayGame);
    
    menuTick(50);
    [self initTimer];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}



-(IBAction)btnUpPressed:(id)sender {
    mBufferedCommand = kCommandUp;
}

-(IBAction)btnDownPressed:(id)sender {
    mBufferedCommand = kCommandDown;
}

-(IBAction)btnLeftPressed:(id)sender {
    mBufferedCommand = kCommandLeft;
    if ((currentGameMenuState == kPlayGame ||
         currentGameMenuState == kBackToGame) &&
        currentPresentationState == kWaitingForInput
        ) {
        
        turnStep = 0;
        turnTarget = 200;
    }
}

-(IBAction)btnRightPressed:(id)sender {
    mBufferedCommand = kCommandRight;
    if ((currentGameMenuState == kPlayGame ||
         currentGameMenuState == kBackToGame) &&
        currentPresentationState == kWaitingForInput
        ) {
        
        turnStep = 200;
        turnTarget = 0;
    }
}


-(IBAction)btnStrafeLeftPressed:(id)sender {
    mBufferedCommand = kCommandStrafeLeft;
}

-(IBAction)btnStrafeRightPressed:(id)sender {
    mBufferedCommand = kCommandStrafeRight;
}


-(IBAction)btnFire1Pressed:(id)sender {
    mBufferedCommand = kCommandFire1;
}

-(IBAction)btnFire2Pressed:(id)sender {
    mBufferedCommand = kCommandFire2;
}

-(IBAction)btnFire3Pressed:(id)sender {
    mBufferedCommand = kCommandFire3;
}

-(IBAction)btnFire4Pressed:(id)sender {
    mBufferedCommand = kCommandFire4;
}

@end
