#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "Engine.h"

#include <stdio.h>
#include <Quickdraw.h>
#include <MacWindows.h>
#include <Dialogs.h>
#include <Menus.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Files.h>
#include <Sound.h>
#include <Processes.h>

#define kMBarDisplayed 128

#define mApple 128
#define mAppleAbout 1

#define mFile 32001
#define mFileReset 1
#define mFileQuit 9

#define rUserAlert 129

#define APP_NAME_STRING "\pSub Mare Imperium - Derelict"
struct RGBColor mPalette[256];
PaletteHandle myPalette;
SndChannelPtr sndChannelPtr;
uint8_t stretchedBuffer[XRES_FRAMEBUFFER*YRES_FRAMEBUFFER];
GWorldPtr offscreenBuffer;
extern enum ECommand mBufferedCommand;
extern char* filePrefix;
extern int visibilityCached;
void soundTick(){}

void playSound(const int) {}

void setupOPL2(void){}

void stopSounds(void) {}


//--------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------
int tick = 0;
WindowPtr mainWindowPtr;
Boolean quit = 0;
int lastTick = 0;

int MouseState = 0;
int HeldPixel = 0;

const short appleM = 0;
const short fileM = 1;
struct BitMap backbuffer;
QDGlobals qdGlobals;
struct Bitmap *mFont;
int mCached;
PixMapHandle pixmap;
uint8_t lastKey = 0;
//--------------------------------------------------------------------------------
Rect TMRGetBounds()
{
	Rect r;

#if TARGET_API_MAC_CARBON
	BitMap 			theScreenBits;
	GetWindowPortBounds(mainWindowPtr, &r);
#else
	r = qd.thePort->portRect;
#endif

	return r;
}
//--------------------------------------------------------------------------------
void DrawWindow() {
	Rect r = TMRGetBounds();
	Rect pixelRect;
	int x = 0, y = 0;
	uint32_t offset = 0;
	const BitMap *memBits;
	const BitMap *winBits;
	GWorldPtr memworld;
	GrafPtr saveport;
	CGrafPtr theport;

	GetPort(&saveport);
	SetPortWindowPort(mainWindowPtr);

	theport = GetWindowPort(mainWindowPtr);
	memworld = (GWorldPtr)GetWRefCon(mainWindowPtr);
	memBits = &((GrafPtr)memworld)->portBits;
	winBits = &mainWindowPtr-> portBits;
	pixelRect.top = 0;
	pixelRect.left = 0;
	pixelRect.bottom = (YRES_FRAMEBUFFER - 1);
	pixelRect.right = (XRES_FRAMEBUFFER - 1);

	r.top = 0;
	r.left = 0;
	r.bottom = 0 + 479;
	r.right = 0 + 639;

	CopyBits(memBits, winBits, &pixelRect, &r, srcCopy, NULL);

	SetPort(saveport);
}
//--------------------------------------------------------------------------------
void DoCommand(long mResult)
{
	short theItem;
	short theMenu;
	Str255		daName;
	short		daRefNum;

	theItem = LoWord(mResult);
	theMenu = HiWord(mResult);

	switch (theMenu) {
		case mApple: {
			if (theItem == mAppleAbout) {
				Alert(rUserAlert, nil);
			} else {
				GetMenuItemText(GetMenuHandle(mApple), theItem, daName);
				daRefNum = OpenDeskAcc(daName);
			}
			break;
		}
		case mFile: {
			switch (theItem) {
				case mFileReset:
					break;

				case mFileQuit:
					quit = 1;
					lastKey = kCommandQuit;
					break;

				default:
					break;
			}
			break;
		}
		default:
			break;
	}
	HiliteMenu(0);
}
//--------------------------------------------------------------------------------
void handleSystemEvents() {
	EventRecord    theEvent;
	WindowPtr whichWindow;
	Rect windRect;
	Boolean        gotevent = 0;


	if (!quit) {
		SystemTask();
		tick++;
		soundTick();

		if (GetNextEvent(everyEvent, &theEvent)) {

			switch (theEvent.what) {

				case mouseUp: {
					MouseState = 0;
					break;
				}

				case mouseDown: {
					switch (FindWindow(theEvent.where, &whichWindow)) {

						// Click happens in a Desk Accessory
						case inSysWindow:  {
							SystemClick(&theEvent, whichWindow);
							break;
						}
						case inMenuBar: {
							DoCommand(MenuSelect(theEvent.where));
							break;
						}
						case inDrag: {
							windRect = qdGlobals.screenBits.bounds;
							DragWindow(whichWindow, theEvent.where, &windRect);
							break;
						}
						case inContent: {
							if (whichWindow != FrontWindow()) {
								SelectWindow(whichWindow);
							} else {
								MouseState = 1;
							}

							break;
						}

						default:
							break;
					}
					break;
				}

				case autoKey:
				case keyDown: {

					char theChar = (theEvent.message&charCodeMask);

					if (theEvent.modifiers&cmdKey) {
						DoCommand(MenuKey(theChar));

					}

					switch (theChar) {
						case 's':
						mBufferedCommand = kCommandStrafeLeft;
						visibilityCached = false;
						break;

						case 'd':
						mBufferedCommand = kCommandStrafeRight;
						visibilityCached = false;
						break;

						case 'q':
						mBufferedCommand = kCommandBack;
						break;

						case 'z':
						mBufferedCommand = kCommandFire1;
						visibilityCached = false;
						break;

						case 'x':
						mBufferedCommand = kCommandFire2;
						visibilityCached = false;
						break;

						case 'c':
						mBufferedCommand = kCommandFire3;
						visibilityCached = false;
						break;
					}
					break;
				}

				case activateEvt: {
					if (theEvent.modifiers&activeFlag) {
					} else {
					}
					break;
				}

				case updateEvt: {
					BeginUpdate((WindowPtr)theEvent.message);
					DrawWindow();
					EndUpdate((WindowPtr)theEvent.message);
					break;
				}
				default:
					break;
			}
		}

		{
			KeyMap keys;

			GetKeys(keys);

			switch(keys[3]) {
				case 64:
				lastKey = kCommandUp;
				break;

				case 32:
				lastKey = kCommandDown;
				break;

				case 16:
				lastKey = kCommandRight;
				break;

				case 8:
				lastKey = kCommandLeft;
				break;

				case 268435456: //enter
				lastKey = kCommandFire1;
				break;

				case 512: //space
				lastKey = kCommandFire2;
				break;
			}

			switch(keys[1]) {
				case 256: //tab
				lastKey = kCommandFire3;
				break;

				case 268435456: //enter
				lastKey = kCommandFire1;
				break;

				case 512: //space
				lastKey = kCommandFire2;
				break;

				case 8192: //ESC
				lastKey = kCommandBack;
				break;
			}

			//cmd + q
			if (keys[0] == 1048576 && keys[1] == 32768) {
				mBufferedCommand = kCommandQuit;
				quit = 1;
				return;
			}

			if (keys[3] == 0 && keys[1] == 0 && lastKey != 0) {
				if (lastKey == kCommandLeft || lastKey == kCommandRight) {
					mTurnBuffer = (enum ECommand)lastKey;
				} else {
					mBufferedCommand = (enum ECommand)lastKey;
				}
				needsToRedrawVisibleMeshes = true;
				visibilityCached = false;
				lastKey = 0;
			}
		}
	}
}
//--------------------------------------------------------------------------------
void SetUpMenus() {
/*
	short i;
	OSErr err;
	long result;
	MenuRef menu;

	MenuHandle myMenus[3];

	myMenus[appleM] = GetMenu(mApple);

	AddResMenu(myMenus[appleM],'DRVR'); // System-provided Desk Accessories menu

	myMenus[appleM] = GetMenu(mApple);
	myMenus[fileM] = GetMenu(mFile);

	for (i = 0; i < 2; i++) {
		InsertMenu(myMenus[i], 0);
	}
*/
	DrawMenuBar();
}
//--------------------------------------------------------------------------------
void graphicsShutdown() {
	ExitToShell();
}
//--------------------------------------------------------------------------------
void flipRenderer() {
	long offset = 0;
	int c;
	Ptr baseAddr;
	long rowBytes;

	renderPageFlip(&stretchedBuffer[0], &framebuffer[0], &previousFrame[0], turnStep, turnTarget, 0);
	pixmap = GetGWorldPixMap(offscreenBuffer);
	LockPixels(pixmap);

	baseAddr = GetPixBaseAddr(pixmap);
	rowBytes = ((*pixmap)->rowBytes & 0x3FFF);

	for (c = 0; c < YRES_FRAMEBUFFER; ++c ) {
		memcpy( baseAddr + offset, &stretchedBuffer[XRES_FRAMEBUFFER * c], XRES_FRAMEBUFFER);
		offset = offset + rowBytes;
	}

	UnlockPixels(pixmap);
	InvalRect(&qdGlobals.thePort->portRect);
}
//--------------------------------------------------------------------------------
void clearRenderer() {

}
//--------------------------------------------------------------------------------
void Initialize(void) {
//	OSErr err;
//	OSErr		error;

	InitGraf((Ptr) &qdGlobals.thePort);
	InitFonts();

	FlushEvents(everyEvent, 0);

	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);

	InitCursor();

	SetUpMenus();
}
//--------------------------------------------------------------------------------
void graphicsInit()
{

	int r, g, b;
	Rect windowRect;
    int pIndex = 0;
   	CTabHandle ctab;
    
	Initialize();
	SetRect(&windowRect, 50, 50, 50 + 640, 50 + 480);
	mainWindowPtr = NewCWindow(nil, &windowRect, APP_NAME_STRING, true, noGrowDocProc, (WindowPtr)-1L, true, (long)nil);
	SetPort((struct GrafPort*)GetWindowPort(mainWindowPtr));
	mCached = false;
    mFont = loadBitmap("font.img");
    defaultFont = mFont;

    myPalette = NewPalette(256, NULL, pmExplicit + pmTolerant, 0);

	for (r = 0; r < 4; ++r) {
		for (g = 0; g < 8; ++g) {
			for (b = 0; b < 8; ++b) {
				mPalette[pIndex].red = ((r * 16) * 0xFFFF) / 64;
				mPalette[pIndex].green = ((g * 8) * 0xFFFF) / 64;
				mPalette[pIndex].blue = ((b * 8) * 0xFFFF) / 64;
				palette[pIndex] = pIndex;
				SetEntryColor(myPalette, pIndex, &mPalette[pIndex]);
				pIndex++;
			}
		}
	}
	ctab = (CTabHandle)NewHandle(sizeof(ColorTable));
	Palette2CTab(myPalette, ctab);
   	CTabChanged(ctab);
	SetPalette((WindowPtr)mainWindowPtr, myPalette, false);
	ActivatePalette(mainWindowPtr);
	AnimatePalette(mainWindowPtr, ctab, 0, 0, 256);

	clearRenderer();

	if (SndNewChannel(&sndChannelPtr, squareWaveSynth, 0, NULL) != noErr ) {
		puts("Error initializing sound");
	}

	SetRect(&windowRect, 0, 0, 640, 480);

	NewGWorld(&offscreenBuffer, 8, &windowRect, ctab, NULL, 0);

	SetWRefCon( mainWindowPtr, (long)offscreenBuffer);
	SetPortWindowPort(mainWindowPtr);
	SelectWindow(mainWindowPtr);
}
