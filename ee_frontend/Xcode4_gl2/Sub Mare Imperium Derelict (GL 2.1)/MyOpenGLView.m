#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else

#include <GL/gl.h>
#include <GL/glu.h>

#endif

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
#include "Renderer.h"
#include "Engine.h"
#include "VisibilityStrategy.h"
#include "PackedFileReader.h"

#import "MyOpenGLView.h"

@implementation MyOpenGLView

void initStation(void);

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

int enable3DRendering = FALSE;
int width = 640;
int height = 480;

float multiplier = 1.0f;
extern uint8_t isRunning;

void enter2D(void);

int submitBitmapToGPU(struct Bitmap *bitmap);

id delegate;
int bufferedInput = -1;

char *textBuffer;
extern char *messageLogBuffer;
extern enum EVisibility *visMap;
extern struct Vec2i *distances;
extern uint8_t *collisionMap;

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

void graphicsInit() {
	defaultFont = NULL;
    enableSmoothMovement = TRUE;
    initGL();
}

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

void graphicsShutdown() {
}

void flipRenderer() {
}

- (void)keyDown:(NSEvent *)event {
    leanY = leanX = 0;
}

- (void)keyUp:(NSEvent *)event {
	bufferedInput = [event keyCode];
}

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (void)repaintGame:(NSTimer *)timer {
	[self setNeedsDisplay:YES];
}



- (void)awakeFromNib {
    GLuint attribs[] =
    {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAStencilSize, 8,
        NSOpenGLPFAAccumSize, 0,
        NSOpenGLPFAOpenGLProfile,
		NSOpenGLProfileVersionLegacy,
        0
    };
    
	NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:(NSOpenGLPixelFormatAttribute *) attribs];
    
	if (!fmt) {
		NSLog(@"No OpenGL pixel format");
	}
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:fmt shareContext:nil];
    
    [self setPixelFormat:fmt];
    
    [self setOpenGLContext:context];
}

- (void) prepareOpenGL
{
	[super prepareOpenGL];
    
	// Synchronize buffer swaps with vertical refresh rate
	GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    [[self openGLContext] makeCurrentContext];
    
    initHW(0, NULL);
	graphicsInit();
    
    enable3DRendering = TRUE;
    enterState(kMainMenu);
    
    [NSTimer scheduledTimerWithTimeInterval:1.0 / 20.0f
									 target:self selector:@selector(repaintGame:) userInfo:nil repeats:YES];

}


- (void)drawRect:(NSRect)rect {
    
    if (!enable3DRendering) {
        return;
    }
    
    [[self openGLContext] makeCurrentContext];
    
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

    isRunning = isRunning && menuTick(20);

	if (!isRunning) {
		exit(0);
	}

	endFrame();

	[[self openGLContext] flushBuffer];
}
@end
