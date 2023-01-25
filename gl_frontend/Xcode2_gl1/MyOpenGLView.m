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

#include "PackedFileReader.h"

#import "MyOpenGLView.h"

@implementation MyOpenGLView

void initStation(void);

NSMutableSet *playingSounds;
int nextAudioChannel = -1;
float multiplier = 1.0f;
extern int isRunning;
NSSound *playerSounds[8];

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


void initHW() {

	NSString *path = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:@"base.pfs"];
	initFileReader([path UTF8String]);


	setupOPL2(0);
}

void graphicsInit() {
	defaultFont = loadBitmap("font.img");
	defaultFont->uploadId = submitBitmapToGPU(defaultFont);
}

void handleSystemEvents() {
	mBufferedCommand = kCommandNone;

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
			mBufferedCommand = kCommandLeft;
			break;

		case 124:
			mBufferedCommand = kCommandRight;
			break;
	}
	bufferedInput = -1;
}

void graphicsShutdown() {
}

void flipRenderer() {
}

- (void)keyDown:(NSEvent *)event {
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

	srand((unsigned int) time(NULL));

	[[self openGLContext] makeCurrentContext];

	initStation();
	initHW(0, NULL);
	initGL();
	graphicsInit();


	enterState(kMainMenu);

	[NSTimer scheduledTimerWithTimeInterval:1.0 / 20.0f
									 target:self selector:@selector(repaintGame:) userInfo:nil repeats:YES];


	return self = [super initWithFrame:frame pixelFormat:[fmt autorelease]];
}

- (void)awakeFromNib {
}

- (void)drawRect:(NSRect)rect {
	startFrameGL((GLsizei) rect.size.width, (GLsizei) rect.size.height);

	isRunning = menuTick(20);

	if (!isRunning) {
		exit(0);
	}

	endFrameGL();

	[[self openGLContext] flushBuffer];
}
@end
