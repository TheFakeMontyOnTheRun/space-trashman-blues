//
//  ViewController.m
//  Sub Mare Imperium: Derelict (GL ES)
//
//  Created by Daniel Monteiro on 12/29/23.
//  Copyright (c) 2023 Daniel Monteiro. All rights reserved.
//

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "Core.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "Engine.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"

#import "ViewController.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

int enable3DRendering = TRUE;
int width = 640;
int height = 480;
char *textBuffer;
int bufferedInput = -1;

extern enum ECommand mBufferedCommand;

void initStation(void);

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;


void shutdownHW(void) {
    
}

void initHW(int argc, char **argv) {
    textBuffer = (char *) allocMem(TEXT_BUFFER_SIZE, GENERAL_MEMORY, 1);
    messageLogBuffer = (char *) allocMem(256, GENERAL_MEMORY, 1);
    collisionMap = (uint8_t *) allocMem(256, GENERAL_MEMORY, 1);
    visMap = (enum EVisibility *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(enum EVisibility), GENERAL_MEMORY, 1);
    distances = (struct Vec2i *) allocMem(2 * MAP_SIZE * MAP_SIZE * sizeof(struct Vec2i), GENERAL_MEMORY, 1);
    itemsInMap = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    map = (uint8_t *) allocMem(MAP_SIZE * MAP_SIZE * sizeof(uint8_t *), GENERAL_MEMORY, 1);
    
	NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"base.pfs"];
	initFileReader([path UTF8String]);
}

@interface ViewController () {
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation ViewController


void graphicsShutdown() {
}

void flipRenderer() {
}

- (void)dealloc
{
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [_context release];

    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    defaultFont = NULL;
    enableSmoothMovement = TRUE;
    
    initStation();
    
    initHW(0, NULL);
    
    [self setupGL];
    
    enterState(kPlayGame);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];
    initGL();
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
    shutdownHW();
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    int rectWidth = rect.size.width;
    int rectHeight = rect.size.height;
    
    if (rectWidth > rectHeight ) {
        float heightBase = rectHeight / (float)height;
        rectWidth = width * heightBase;
        rectHeight = height * heightBase;
        /* height is the base, since width is too big and we must add borders*/
    } else {
        /* width is the base, since height is too big and we must add borders*/
        float widthBase = rectWidth / (float) width;
        rectWidth = width * widthBase;
        rectHeight = height * widthBase;
    }
    
    
	startFrame( (rect.size.width - rectWidth) / 2,
                 (rect.size.height - rectHeight) / 2,
                 rectWidth,
                 rectHeight);
    menuTick(10);
    endFrame();
    flipRenderer();
}



-(IBAction)btnUpPressed:(id)sender {
    mBufferedCommand = kCommandUp;
}

-(IBAction)btnDownPressed:(id)sender {
    mBufferedCommand = kCommandDown;
}

-(IBAction)btnLeftPressed:(id)sender {
    turning = 1;
    leanX = -ANGLE_TURN_STEP;
}

-(IBAction)btnRightPressed:(id)sender {
    turning = 1;
    leanX = ANGLE_TURN_STEP;
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

void handleSystemEvents() {
	
    
	switch (bufferedInput) {
		case 0: //a
		case 36: //enter
			mBufferedCommand = kCommandFire1;
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
			break;
		case 125:
			mBufferedCommand = kCommandDown;
			break;
            
		case 123:
            turning = 1;
            leanX = -ANGLE_TURN_STEP;
			break;
            
		case 124:
            turning = 1;
            leanX = ANGLE_TURN_STEP;
			break;
	}
	bufferedInput = -1;
}
