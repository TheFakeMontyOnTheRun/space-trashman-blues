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

extern void REGARGS c2p1x1_4_c5_bm(
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


struct Bitmap *defaultFont;
int dirtyLineY0 = 0;
int dirtyLineY1 = YRES_FRAMEBUFFER;
struct Vec2i *distances;
enum EVisibility *visMap;
int enable3DRendering = 0;

uint8_t framebuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

uint8_t enableSmoothMovement = 1;
uint8_t *collisionMap;
int currentSelectedItem = 0;
enum ECommand mBufferedCommand;
struct Bitmap *mapTopLevel = NULL;
char *messageLogBuffer;
int needsToRedrawVisibleMeshes = TRUE;
uint8_t texturesUsed = 0;
const char *thisMissionName;
int turnStep = 0;
int turnTarget = 0;
int visibilityCached = FALSE;

struct NewScreen xnewscreen = {
        0,			  /* LeftEdge*/
        0,			  /* TopEdge   */
        320,		  /* Width     */
        200,		  /* Height    */
        4,			  /* Depth   */
        0,			  /* DetailPen */
        1,			  /* BlockPen */
        0,			  /* ViewModes High-resolution, Interlaced */
        CUSTOMSCREEN,	  /* Type customized screen. */
        NULL,		  /* Font */
        "The Mistral Report", /* Title */
        NULL,		  /* Gadget */
        NULL		  /* BitMap */
};

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
        (UBYTE * ) "Sub Mare Imperium - Derelict",/* Title       */
        NULL,                          /* Screen      */
        NULL,                          /* BitMap      */
        320,                          /* MinWidth    */
        200,                          /* MinHeight   */
        320,                          /* MaxWidth    */
        200,                          /* MaxHeight   */
        CUSTOMSCREEN                      /* Type */
};

enum ECommand getInput(void) {
    const enum ECommand toReturn = mBufferedCommand;
    mBufferedCommand = kCommandNone;
    return toReturn;
}

void graphicsShutdown() {
    ClearPointer(my_window);
    CloseWindow(my_window);
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
}

void clearGraphics(void) {
    memset(framebuffer, 0, XRES_FRAMEBUFFER * YRES_FRAMEBUFFER);
}


void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    if (colour >= 16) {
        if ((x + y) & 1) {
            framebuffer[(XRES_FRAMEBUFFER * y) + x] = 0;
        } else {
            framebuffer[(XRES_FRAMEBUFFER * y) + x] = colour - 16;
        }
    } else {
        framebuffer[(XRES_FRAMEBUFFER * y) + x] = colour;
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {
    uint8_t *ptr;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (x0 < 0 || x0 >= XRES_FRAMEBUFFER) {
        return;
    }

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 >= YRES_FRAMEBUFFER || _y1 < 0) {
        return;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= YRES_FRAMEBUFFER) {
        _y1 = YRES_FRAMEBUFFER - 1;
    }

    ptr = &framebuffer[(XRES_FRAMEBUFFER * _y0) + (x0)];

    if (colour <= 16) {
        for (int16_t y = _y0; y <= _y1; ++y) {
            *ptr = colour;
            ptr += XRES_FRAMEBUFFER;
        }
    } else {
        colour = colour - 16;
        uint8_t stipple = ((x0 + y0) & 1);

        for (int16_t y = _y0; y <= _y1; ++y) {
            if (stipple = ~stipple) {
                *ptr = colour;
            } else {
                *ptr = 0;
            }

            ptr += XRES_FRAMEBUFFER;
        }
    }
}

void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0) {
        return;
    }

    int16_t _x0 = x0;
    int16_t _x1 = x1;

    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }

    if (_x0 < 0) {
        _x0 = 0;
    }

    if (_x1 >= XRES_FRAMEBUFFER) {
        _x1 = XRES_FRAMEBUFFER - 1;
    }

    if (colour <= 16) {
        uint8_t *ptr = &framebuffer[(XRES_FRAMEBUFFER * y) + _x0];
        for (int16_t x = _x0; x <= _x1; ++x) {
            *ptr++ = colour;
        }
    } else {
        colour = colour - 16;
        uint8_t stipple = ((x0 + y) & 1);
        uint8_t *ptr = &framebuffer[(XRES_FRAMEBUFFER * y) + _x0];
        for (int16_t x = _x0; x <= _x1; ++x) {
            if (stipple = ~stipple) {
                *ptr++ = colour;
            } else {
                *ptr++ = 0;
            }
        }
    }
}

void querySoundDriver() {
    setupOPL2(1);
}

struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

uint8_t getPaletteEntry ( uint32_t origin ) {
    uint8_t shade;

    if ( !( origin & 0xFF000000 ) ) {
        return TRANSPARENCY_COLOR;
    }

    shade = 0;
    shade += ( ( ( ( ( origin & 0x0000FF )       ) << 1 ) >> 8 ) ) << 0;
    shade += ( ( ( ( ( origin & 0x00FF00 ) >>  8 ) << 1 ) >> 8 ) ) << 1;
    shade += ( ( ( ( ( origin & 0xFF0000 ) >> 16 ) << 1 ) >> 8 ) ) << 2;

    return shade & 15;
}

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

    SetRGB4(&screen->ViewPort, 0, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0x00));
    SetRGB4(&screen->ViewPort, 1, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0xAA));
    SetRGB4(&screen->ViewPort, 2, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0x00));
    SetRGB4(&screen->ViewPort, 3, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0xAA));
    SetRGB4(&screen->ViewPort, 4, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0x00));
    SetRGB4(&screen->ViewPort, 5, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0xAA));
    SetRGB4(&screen->ViewPort, 6, NORMALIZE(0xAA), NORMALIZE(0x55), NORMALIZE(0x00));
    SetRGB4(&screen->ViewPort, 7, NORMALIZE(0xAA), NORMALIZE(0xAA), NORMALIZE(0xAA));
    SetRGB4(&screen->ViewPort, 8, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0x55));
    SetRGB4(&screen->ViewPort, 9, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0xFF));
    SetRGB4(&screen->ViewPort, 10, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0x55));
    SetRGB4(&screen->ViewPort, 11, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0xFF));
    SetRGB4(&screen->ViewPort, 12, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0x55));
    SetRGB4(&screen->ViewPort, 13, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0xFF));
    SetRGB4(&screen->ViewPort, 14, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0x55));
    SetRGB4(&screen->ViewPort, 15, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0xFF));

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
    c2p1x1_4_c5_bm(XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, 0, 0, &framebuffer[0], my_window->RPort->BitMap);

}
