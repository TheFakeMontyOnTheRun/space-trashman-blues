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
#include "PackedFileReader.h"
#include "Vec.h"

#include "CTile3DProperties.h"

#include "Core.h"
#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "Globals.h"

enum ESoundDriver soundDriver = kNoSound;
uint8_t finalBuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

#define NORMALIZE(x) (((x * 16) / 256))

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs

#ifndef CD32
extern void REGARGS
c2p1x1_8_c5_bm(
REG(d0, UWORD chunky_x),
REG(d1, UWORD chunky_y),
REG(d2, UWORD offset_x),
REG(d3, UWORD offset_y),
REG(a0, UBYTE * chunky_buffer),
REG(a1, struct BitMap *bitmap)
);
#endif

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen *screen;

#ifdef CD32
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
        320,          /* Width    */
        200,          /* Height   */
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

void openCOM() {}

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
    setupOPL2(1);
}

struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

#ifdef CD32
uint8_t getPaletteEntry ( uint32_t origin ) {
  uint8_t shade;

  if ( !( origin & 0xFF000000 ) ) {
    return TRANSPARENCY_COLOR;
  }

  shade = 0;
  shade += ( ( ( ( ( origin & 0x0000FF ) ) << 2 ) >> 8 ) ) << 4;
  shade += ( ( ( ( ( origin & 0x00FF00 ) >> 8 ) << 2 ) >> 8 ) ) << 2;
  shade += ( ( ( ( ( origin & 0xFF0000 ) >> 16 ) << 1 ) >> 8 ) ) << 0;

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

    my_window = (struct Window *) OpenWindowTags(NULL,
                                                 WA_Left, 0,
                                                 WA_Top, 0,
                                                 WA_Width, 320,
                                                 WA_Height, 200,
                                                 WA_IDCMP,
                                                 ACTIVEWINDOW | VANILLAKEY | CLOSEWINDOW | RAWKEY,
                                                 WA_Flags, SMART_REFRESH | WINDOWDRAG | WINDOWDEPTH | ACTIVATE,
                                                 WA_CustomScreen, (ULONG)screen,
                                                 TAG_DONE);

    if (my_window == NULL) {
        puts("nope 2!");
        CloseLibrary((struct Library *) IntuitionBase);
        exit(0);
    }

#ifdef CD32
    SetRGB4 ( &screen->ViewPort, 0, NORMALIZE(0), NORMALIZE(0), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 1, NORMALIZE(0), NORMALIZE(0), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 2, NORMALIZE(0), NORMALIZE(0), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 3, NORMALIZE(0), NORMALIZE(0), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 4, NORMALIZE(0), NORMALIZE(64), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 5, NORMALIZE(0), NORMALIZE(64), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 6, NORMALIZE(0), NORMALIZE(64), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 7, NORMALIZE(0), NORMALIZE(64), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 8, NORMALIZE(0), NORMALIZE(128), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 9, NORMALIZE(0), NORMALIZE(128), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 10, NORMALIZE(0), NORMALIZE(128), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 11, NORMALIZE(0), NORMALIZE(128), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 12, NORMALIZE(0), NORMALIZE(192), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 13, NORMALIZE(0), NORMALIZE(192), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 14, NORMALIZE(0), NORMALIZE(192), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 15, NORMALIZE(0), NORMALIZE(192), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 16, NORMALIZE(128), NORMALIZE(0), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 17, NORMALIZE(128), NORMALIZE(0), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 18, NORMALIZE(128), NORMALIZE(0), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 19, NORMALIZE(128), NORMALIZE(0), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 20, NORMALIZE(128), NORMALIZE(64), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 21, NORMALIZE(128), NORMALIZE(64), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 22, NORMALIZE(128), NORMALIZE(64), NORMALIZE(128) );
    SetRGB4 ( &screen->ViewPort, 23, NORMALIZE(128), NORMALIZE(64), NORMALIZE(192) );
    SetRGB4 ( &screen->ViewPort, 24, NORMALIZE(128), NORMALIZE(128), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 25, NORMALIZE(128), NORMALIZE(128), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 26,NORMALIZE( 128), NORMALIZE(128), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 27,NORMALIZE( 128), NORMALIZE(128), NORMALIZE(192 ));
    SetRGB4 ( &screen->ViewPort, 28, NORMALIZE(128), NORMALIZE(192), NORMALIZE(0 ));
    SetRGB4 ( &screen->ViewPort, 29, NORMALIZE(128), NORMALIZE(192), NORMALIZE(64 ));
    SetRGB4 ( &screen->ViewPort, 30, NORMALIZE(128), NORMALIZE(192), NORMALIZE(128 ));
    SetRGB4 ( &screen->ViewPort, 31, NORMALIZE(128), NORMALIZE(192), NORMALIZE(192 ));
#else
    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palete[paletteEntry].r = ((((16 * (b - 0x38)) / 256)));
                palete[paletteEntry].g = ((((16 * (g - 0x18)) / 256)));
                palete[paletteEntry].b = ((((16 * (r - 0x10)) / 256)));
            }
        }
    }

    for (c = 0; c < 256; ++c) {
        SetRGB4(&screen->ViewPort, c, palete[c].r, palete[c].g,
                palete[c].b);
    }


#endif
    enableSmoothMovement = TRUE;
    
    SetPointer(my_window, emptypointer, 1, 16, 0, 0);

    querySoundDriver();
    defaultFont = loadBitmap("font.img");
}

/*
 * Code lifted (and heavily modified) from the Strife AGA port by Lantus
 * https://github.com/lantus/Strife/blob/master/i_video.c
 * */
int xlate_key(UWORD rawkey, UWORD qualifier, APTR eventptr) {
    char buffer[4], c;
    struct InputEvent ie;

    if (rawkey < 0x40) {
        ie.ie_Class = IECLASS_RAWKEY;
        ie.ie_Code = rawkey;
        ie.ie_Qualifier = qualifier;
        ie.ie_EventAddress = eventptr;

        if (MapRawKey(&ie, buffer, sizeof(buffer), NULL) > 0) {
            c = buffer[0];
            if (c >= '0' && c <= '9')       /* numeric pad */
                switch (c) {
                    case '4':
                        mBufferedCommand = kCommandLeft;
                        visibilityCached = FALSE;
                        break;
                    case '5':
                        mBufferedCommand = kCommandDown;
                        visibilityCached = FALSE;
                        break;
                    case '6':
                        mBufferedCommand = kCommandRight;
                        visibilityCached = FALSE;
                        break;
                    case '8':
                        mBufferedCommand = kCommandUp;
                        visibilityCached = FALSE;
                        break;
                }
        }
    } else {
        switch (rawkey) {
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
                break;
            case 0x4F:
                mBufferedCommand = kCommandLeft;
                visibilityCached = FALSE;
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

        if (messageClass != VANILLAKEY && code != 0) {
            if ((code & 0x80) == 0) {
                xlate_key(code, my_message->Qualifier, my_message->IAddress);
            }
        }

        ReplyMsg((struct Message *) my_message);

        if (messageClass == VANILLAKEY) {

            switch (code) {
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

                case 'b':
                    mBufferedCommand = kCommandLeft;
                    visibilityCached = FALSE;
                    break;

                case 'm':
                    mBufferedCommand = kCommandRight;
                    visibilityCached = FALSE;
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
#ifdef EMIT_QUIT_OPTION
                case 'l':
                    mBufferedCommand = kCommandQuit;
                    break;
#endif
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
        }
    }
}

void flipRenderer() {
    renderPageFlip(&finalBuffer[0], framebuffer,
                   previousFrame, turnStep, turnTarget, 0);


#ifdef CD32

    WriteChunkyPixels(my_window->RPort, 0, 0, 320, 200, &finalBuffer[0], 320);

#else

#ifdef AGA8BPP
    OwnBlitter();
    WaitBlit();
    c2p1x1_8_c5_bm(320,200,0,0,&finalBuffer[0], my_window->RPort->BitMap);
    DisownBlitter();
#else
    WriteChunkyPixels(my_window->RPort, 0, 0, 319, 199, &finalBuffer[0], 320);
#endif

#endif

}

void clear() {}
