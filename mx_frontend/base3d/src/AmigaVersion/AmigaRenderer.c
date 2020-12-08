#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <intuition/intuition.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "AmigaInt.h"
#include "SoundSystem.h"
#include "Enums.h"
#include "FixP.h"
#include "Common.h"
#include "Vec.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "CPackedFileReader.h"
#include "Vec.h"

#include "CTile3DProperties.h"

#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs

extern void REGARGS c2p1x1_8_c5_bm(
REG(d0, UWORD chunky_x),
REG(d1, UWORD chunky_y),
REG(d2, UWORD offset_x),
REG(d3, UWORD offset_y),
REG(a0, UBYTE *chunky_buffer),
REG(a1, struct BitMap *bitmap));

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen *screen;

#ifdef AGA5BPP
struct NewScreen xnewscreen = {
	0,			  /* LeftEdge*/
	0,			  /* TopEdge   */
	320,		  /* Width     */
	200,		  /* Height    */
	5,			  /* Depth   */
	0,			  /* DetailPen */
	1,			  /* BlockPen */
	0,			  /* ViewModes High-resolution, Interlaced */
	CUSTOMSCREEN,	  /* Type customized screen. */
	NULL,		  /* Font */
	"The Mistral Report", /* Title */
	NULL,		  /* Gadget */
	NULL		  /* BitMap */
};
#else
struct NewScreen xnewscreen = {
		0,              /* LeftEdge */
		0,              /* TopEdge  */
		640,          /* Width    */
		256,          /* Height   */
		8,              /* Depth    */
		0,              /* DetailPen */
		1,              /* BlockPen */
		0,              /* ViewModes High-resolution, Interlaced */
		CUSTOMSCREEN,      /* Type customized screen. */
		NULL,          /* Font */
		"The Mistral Report", /* Title */
		NULL,          /* Gadget */
		NULL          /* BitMap */
};
#endif

	struct NewWindow my_new_window = {
			0,                              /* LeftEdge*/
			0,                              /* TopEdge*/
			320,                          /* Width */
			200,                          /* Height */
			0,                              /* DetailPen  */
			1,                              /* BlockPen   */
			ACTIVEWINDOW | VANILLAKEY | CLOSEWINDOW | RAWKEY, /* IDCMPFlags  */
			SMART_REFRESH |                      /* Flags       */
			WINDOWDRAG |                      /*             */
			WINDOWDEPTH |                      /*             */
			ACTIVATE,                      /*            */
			NULL,                          /* FirstGadget */
			NULL,                          /* CheckMark   */
			(UBYTE *) "The Mistral Report",              /* Title       */
			NULL,                          /* Screen      */
			NULL,                          /* BitMap      */
			320,                          /* MinWidth    */
			200,                          /* MinHeight   */
			320,                          /* MaxWidth    */
			200,                          /* MaxHeight   */
			CUSTOMSCREEN                      /* Type */
	};

void openCOM() {}

uint8_t turnBuffer[320 * 200];

uint8_t readByte() {
	return kCommandNone;
}

void writeByte(uint8_t command) {}

long frame = 0;

void graphicsShutdown() {
	ClearPointer(my_window);
	CloseWindow(my_window);
	CloseScreen(screen);
	CloseLibrary((struct Library *) IntuitionBase);
}

void putStr(int x, int y, const char *str, int fg, int bg) {}

void drawTitleBox() {}

void querySoundDriver() {
	setupOPL2();
}

struct RGB8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

#ifdef AGA5BPP
uint8_t getPaletteEntry ( uint32_t origin ) {
  uint8_t shade;

  if ( !( origin & 0xFF000000 ) ) {
	return TRANSPARENCY_COLOR;
  }

  shade = 0;
  shade += ( ( ( ( ( origin & 0x0000FF ) ) << 2 ) >> 8 ) ) << 0;
  shade += ( ( ( ( ( origin & 0x00FF00 ) >> 8 ) << 2 ) >> 8 ) ) << 2;
  shade += ( ( ( ( ( origin & 0xFF0000 ) >> 16 ) << 1 ) >> 8 ) ) << 4;

  return shade;
}
#else

uint8_t getPaletteEntry(uint32_t origin) {
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

#endif

/*
 * Code lifted (and heavily modified) from the Strife AGA port by Lantus
 * https://github.com/lantus/Strife/blob/master/i_video.c
 * */

static UWORD emptypointer[] = {
		0x0000, 0x0000,    /* reserved, must be NULL */
		0x0000, 0x0000,     /* 1 row of image data */
		0x0000, 0x0000    /* reserved, must be NULL */
};

void graphicsInit() {
	int r, g, b;
	int c;
	struct RGB8 palete[256];
	struct ColorMap *cm;
	struct Window *window;
	struct IntuiMessage *msg;
	struct DisplayInfo displayinfo;
	struct TagItem taglist[3];
	int OpenA2024 = FALSE;
	int IsV36 = FALSE;
	int IsPAL;

	drawTitleBox();

	IntuitionBase =
			(struct IntuitionBase *) OpenLibrary("intuition.library", 0);

	if (IntuitionBase == NULL) {
		puts("nope 1!");
		exit(0);
	}

	if ((screen = OpenScreen(&xnewscreen)) == NULL) {
	}

	my_new_window.Screen = screen;

	my_window = (struct Window *) OpenWindow(&my_new_window);

	if (my_window == NULL) {
		puts("nope 2!");
		CloseLibrary((struct Library *) IntuitionBase);
		exit(0);
	}

#ifdef AGA5BPP
	SetRGB4 ( &screen->ViewPort, 0, 0, 0, 0 );
	SetRGB4 ( &screen->ViewPort, 1, 0, 0, 64 );
	SetRGB4 ( &screen->ViewPort, 2, 0, 0, 128 );
	SetRGB4 ( &screen->ViewPort, 3, 0, 0, 192 );
	SetRGB4 ( &screen->ViewPort, 4, 0, 64, 0 );
	SetRGB4 ( &screen->ViewPort, 5, 0, 64, 64 );
	SetRGB4 ( &screen->ViewPort, 6, 0, 64, 128 );
	SetRGB4 ( &screen->ViewPort, 7, 0, 64, 192 );
	SetRGB4 ( &screen->ViewPort, 8, 0, 128, 0 );
	SetRGB4 ( &screen->ViewPort, 9, 0, 128, 64 );
	SetRGB4 ( &screen->ViewPort, 10, 0, 128, 128 );
	SetRGB4 ( &screen->ViewPort, 11, 0, 128, 192 );
	SetRGB4 ( &screen->ViewPort, 12, 0, 192, 0 );
	SetRGB4 ( &screen->ViewPort, 13, 0, 192, 64 );
	SetRGB4 ( &screen->ViewPort, 14, 0, 192, 128 );
	SetRGB4 ( &screen->ViewPort, 15, 0, 192, 192 );
	SetRGB4 ( &screen->ViewPort, 16, 128, 0, 0 );
	SetRGB4 ( &screen->ViewPort, 17, 128, 0, 64 );
	SetRGB4 ( &screen->ViewPort, 18, 128, 0, 128 );
	SetRGB4 ( &screen->ViewPort, 19, 128, 0, 192 );
	SetRGB4 ( &screen->ViewPort, 20, 128, 64, 0 );
	SetRGB4 ( &screen->ViewPort, 21, 128, 64, 64 );
	SetRGB4 ( &screen->ViewPort, 22, 128, 64, 128 );
	SetRGB4 ( &screen->ViewPort, 23, 128, 64, 192 );
	SetRGB4 ( &screen->ViewPort, 24, 128, 128, 0 );
	SetRGB4 ( &screen->ViewPort, 25, 128, 128, 64 );
	SetRGB4 ( &screen->ViewPort, 26, 128, 128, 128 );
	SetRGB4 ( &screen->ViewPort, 27, 128, 128, 192 );
	SetRGB4 ( &screen->ViewPort, 28, 128, 192, 0 );
	SetRGB4 ( &screen->ViewPort, 29, 128, 192, 64 );
	SetRGB4 ( &screen->ViewPort, 30, 128, 192, 128 );
	SetRGB4 ( &screen->ViewPort, 31, 128, 192, 192 );
#else
	for (r = 0; r < 256; r += 16) {
		for (g = 0; g < 256; g += 8) {
			for (b = 0; b < 256; b += 8) {
				uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
				uint8_t paletteEntry = getPaletteEntry(pixel);
				palete[paletteEntry].r = ((((16 * (b - 0x38 )) / 256) ) );
				palete[paletteEntry].g = ((((16 * (g - 0x18 )) / 256) ) );
				palete[paletteEntry].b = ((((16 * (r - 0x10 )) / 256) ) );
			}
		}
	}

	for (c = 0; c < 256; ++c) {
		SetRGB4(&screen->ViewPort, c, palete[c].r, palete[c].g,
				palete[c].b);
	}
#endif

	SetPointer (my_window, emptypointer, 1, 16, 0, 0);

	querySoundDriver();
	defaultFont = loadBitmap("font.img");
}

/*
 * Code lifted (and heavily modified) from the Strife AGA port by Lantus
 * https://github.com/lantus/Strife/blob/master/i_video.c
 * */
int xlate_key (UWORD rawkey, UWORD qualifier, APTR eventptr)
{
	char buffer[4], c;
	struct InputEvent ie;

	if (rawkey < 0x40) {
		ie.ie_Class = IECLASS_RAWKEY;
		ie.ie_Code = rawkey;
		ie.ie_Qualifier = qualifier;
		ie.ie_EventAddress = eventptr;

		if (MapRawKey (&ie, buffer, sizeof(buffer), NULL) > 0) {
			c = buffer[0];
			if (c >= '0' && c <= '9')       /* numeric pad */
				switch (c) {
					case '4':
						mBufferedCommand = kCommandLeft;
						visibilityCached = FALSE;
                        turnStep = 0;
                        turnTarget = 256;
						break;
					case '5':
						mBufferedCommand = kCommandDown;
						visibilityCached = FALSE;
						break;
					case '6':
						mBufferedCommand = kCommandRight;
						visibilityCached = FALSE;
                        turnStep = 256;
                        turnTarget = 0;
						break;
					case '8':
						mBufferedCommand = kCommandUp;
						visibilityCached = FALSE;
						break;
				}
			}
	} else {
		switch(rawkey) {
			case 0x4C:
				mBufferedCommand = kCommandUp;
				visibilityCached = FALSE;
				break;
			case 0x4D:
				mBufferedCommand = kCommandDown;
				visibilityCached = FALSE;
				break;
			case 0x4E:
				mBufferedCommand = kCommandRight;
				visibilityCached = FALSE;
                turnStep = 256;
                turnTarget = 0;
                break;
			case 0x4F:
				mBufferedCommand = kCommandLeft;
				visibilityCached = FALSE;
                turnStep = 0;
                turnTarget = 256;
				break;
			case 96:
			case 97:
				mBufferedCommand = kCommandFire3;
				break;
		}
	}
}

/*Same as above*/
void handleSystemEvents() {

	struct IntuiMessage *my_message;
	ULONG messageClass;
	USHORT code;

	if (my_message = (struct IntuiMessage *) GetMsg(my_window->UserPort)) {
		int handled = FALSE;
		messageClass = my_message->Class;
		code = my_message->Code;

		if (messageClass != VANILLAKEY && code != 0 ) {
			if ((code & 0x80) == 0) {
				xlate_key(code, my_message->Qualifier, my_message->IAddress);
			}
		}

		ReplyMsg ((struct Message *)my_message);

		if (messageClass == VANILLAKEY) {

			switch(code) {
				case 27:
					handled = TRUE;
					mBufferedCommand = kCommandBack;
					break;
			}
		}

		if (messageClass == VANILLAKEY && !handled) {
			switch (code) {
				case 'q':
					mBufferedCommand = kCommandBack;
					break;

				case '\n':
				case '\r':
				case 'i':
				case 'z':
					mBufferedCommand = kCommandFire1;
					break;

			    case 'a':
                    mBufferedCommand = kCommandFire4;
                    break;
			    case 'v':
                    mBufferedCommand = kCommandFire5;
                    break;
			    case 'f':
                    mBufferedCommand = kCommandFire6;
                    break;

				case 'b':
					mBufferedCommand = kCommandLeft;
					visibilityCached = FALSE;
					turnStep = 0;
					turnTarget = 256;
					break;

				case 'm':
					mBufferedCommand = kCommandRight;
					visibilityCached = FALSE;
					turnStep = 256;
					turnTarget = 0;
					break;

				case 'h':
					mBufferedCommand = kCommandUp;
					visibilityCached = FALSE;
					break;

				case 's':
					mBufferedCommand = kCommandStrafeLeft;
					break;
				case 'd':
					mBufferedCommand = kCommandStrafeRight;
					break;

				case 'l':
					mBufferedCommand = kCommandQuit;
					break;

				case 'n':
					mBufferedCommand = kCommandDown;
					visibilityCached = FALSE;
					break;

				case 'e':
					break;

				case 'o':
				case 'x':
				case ' ':
					mBufferedCommand = kCommandFire2;
					break;

				case 'p':
				case 'c':
					mBufferedCommand = kCommandFire3;
					break;
			}

            if (mBufferedCommand != kCommandLeft && mBufferedCommand != kCommandRight && mBufferedCommand != kCommandNone) {
                turnStep = 0;
                turnTarget = 0;
            }
		}
	}
}

void flipRenderer() {



    int x, y;



    if ( turnTarget == turnStep ) {
#ifdef CD32
        WriteChunkyPixels(my_window->RPort, 0, 0, 320, 200, &framebuffer[0], 320);
#else
#ifdef AGA8BPP
        c2p1x1_8_c5_bm(320,200,0,0,&framebuffer[0], my_window->RPort->BitMap);
#else
        WriteChunkyPixels(my_window->RPort, 0, 0, 319, 199, &framebuffer[0], 320);
#endif
#endif
        memcpy( previousFrame, framebuffer, 320 * 200);
    } else if ( turnStep < turnTarget ) {

        uint8_t *bufferPtr = &turnBuffer[0];
        for ( y = 0; y < 200; ++y ) {
            for ( x = 0; x < 320; ++x ) {
                uint8_t index;

                if (x < 256 && y >= 8  ) {

                    if ( x  >= turnStep ) {
                        index = previousFrame[ (320 * y) - turnStep + x ];
                    } else {
                        index = framebuffer[ (320 * y) + x - 64 - turnStep];
                    }
                } else {
                    index = framebuffer[ (320 * y) + x];
                }

                *bufferPtr = index;
                ++bufferPtr;
            }
        }

        turnStep+= 32;
#ifdef CD32
        WriteChunkyPixels(my_window->RPort, 0, 0, 320, 200, &turnBuffer[0], 320);
#else
#ifdef AGA8BPP
        c2p1x1_8_c5_bm(320,200,0,0,&turnBuffer[0], my_window->RPort->BitMap);
#else
        WriteChunkyPixels(my_window->RPort, 0, 0, 319, 199, &turnBuffer[0], 320);
#endif
#endif
    } else {

        uint8_t *bufferPtr = &turnBuffer[0];
        for ( y = 0; y < 200; ++y ) {
            for ( x = 0; x < 320; ++x ) {
                uint8_t index;

                if (x < 256 && y >= 8  ) {

                    if ( x  >= turnStep ) {
                        index = framebuffer[ (320 * y) - turnStep + x ];
                    } else {
                        index = previousFrame[ (320 * y) + x - 64 - turnStep];
                    }

                } else {
                    index = framebuffer[ (320 * y) + x];
                }

                *bufferPtr = index;
                ++bufferPtr;
            }
        }

        turnStep-= 32;
#ifdef CD32
        WriteChunkyPixels(my_window->RPort, 0, 0, 320, 200, &turnBuffer[0], 320);
#else
#ifdef AGA8BPP
        c2p1x1_8_c5_bm(320,200,0,0,&turnBuffer[0], my_window->RPort->BitMap);
#else
        WriteChunkyPixels(my_window->RPort, 0, 0, 319, 199, &turnBuffer[0], 320);
#endif
#endif
    }
}

void clear() {}