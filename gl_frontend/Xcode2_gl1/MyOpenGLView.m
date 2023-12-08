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
#include "CRenderer.h"
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

NSMutableSet *playingSounds;
int nextAudioChannel = -1;
float multiplier = 1.0f;
extern uint8_t isRunning;
NSSound *playerSounds[8];
enum ESoundDriver soundDriver = kNoSound;

void enter2D(void);

int submitBitmapToGPU(struct Bitmap *bitmap);

NSString *fileNamesForSounds[] = {
		@"menu_move",
		@"menu_select",
		@"gotclue",
		@"detected2",
		@"bong",
		@"fire",
		@"enemyfire",
		@"detected2"
};

id delegate;
int bufferedInput = -1;

void setupOPL2(int port) {
	int c;
	NSBundle *bundle = [NSBundle mainBundle];
	playingSounds = [[NSMutableSet alloc] init];

	for (c = 0; c < 8; ++c) {
		NSString *path = [[NSURL fileURLWithPath:[bundle pathForResource:fileNamesForSounds[c] ofType:@"wav"]] path];
		playerSounds[c] = [[NSSound alloc] initWithContentsOfFile:path byReference:NO];
		[playerSounds[c] setDelegate:delegate];
	}
}

void stopSounds(void) {}

void playSound(const int action) {
	NSSound *original = playerSounds[action];
	NSSound *newSound = (NSSound * )
	[original copy];
	[playingSounds addObject:newSound];
	[newSound play];
}

- (void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool {
	[sound release];
	[playingSounds removeObject:sound];
}

void soundTick(void) {}

void muteSound(void) {}

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


	setupOPL2(0);
}

void graphicsInit() {
	defaultFont = loadBitmap("font.img");
	defaultFont->uploadId = submitBitmapToGPU(defaultFont);
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

- (id)initWithFrame:(NSRect)frame {
	GLuint attribs[] =
			{
					NSOpenGLPFANoRecovery,
					NSOpenGLPFAWindow,
					NSOpenGLPFAAccelerated,
					NSOpenGLPFADoubleBuffer,
					NSOpenGLPFAColorSize, 24,
					NSOpenGLPFAAlphaSize, 8,
					NSOpenGLPFADepthSize, 24,
					NSOpenGLPFAStencilSize, 8,
					NSOpenGLPFAAccumSize, 0,
					0
			};

	NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:(NSOpenGLPixelFormatAttribute *) attribs];

	if (!fmt) {
		NSLog(@"No OpenGL pixel format");
	}

	[[self openGLContext] makeCurrentContext];

	initHW(0, NULL);
	graphicsInit();


	enterState(kMainMenu);

	[NSTimer scheduledTimerWithTimeInterval:1.0 / 20.0f
									 target:self selector:@selector(repaintGame:) userInfo:nil repeats:YES];


	return self = [super initWithFrame:frame pixelFormat:[fmt autorelease]];
}

- (void)awakeFromNib {
}

- (void)drawRect:(NSRect)rect {
    int width = rect.size.width;
    int height = rect.size.height;
    
    if (width > height ) {
        float heightBase = height / 240.0f;
        width = 320 * heightBase;
        height = 240 * heightBase;
        /* height is the base, since width is too big and we must add borders*/
    } else {
        /* width is the base, since height is too big and we must add borders*/
        float widthBase = width / 320.0f;
        width = 320 * widthBase;
        height = 240 * widthBase;
    }
    
    
    /* Ugly hack to prevent garbage from appearing on the borders */
	startFrameGL( 0, 0, rect.size.width, rect.size.height);
    fillRect(0, 0, 320, 200, getPaletteEntry(0xFF000000), FALSE);
    
    
	startFrameGL( (rect.size.width - width) / 2,
                 (rect.size.height - height) / 2,
                 width,
                 height);

    isRunning = isRunning && menuTick(20);

	if (!isRunning) {
		exit(0);
	}

	endFrameGL();

	[[self openGLContext] flushBuffer];
}
@end
