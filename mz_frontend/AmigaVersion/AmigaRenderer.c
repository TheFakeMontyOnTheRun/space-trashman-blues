#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <intuition/intuition.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "AmigaInt.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs

#ifndef CD32
extern void REGARGS c2p1x1_4_c5_bm(
REG(d0, UWORD chunky_x),
REG(d1, UWORD chunky_y),
REG(d2, UWORD offset_x),
REG(d3, UWORD offset_y),
REG(a0, UBYTE *chunky_buffer),
REG(a1, struct BitMap *bitmap));
#endif

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen *screen;

uint8_t *framebuffer;
uint8_t bufferInput = '.';

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
        (UBYTE * ) "The Mistral Report",              /* Title       */
        NULL,                          /* Screen      */
        NULL,                          /* BitMap      */
        320,                          /* MinWidth    */
        200,                          /* MinHeight   */
        320,                          /* MaxWidth    */
        200,                          /* MaxHeight   */
        CUSTOMSCREEN                      /* Type */
};

long frame = 0;

void putStr(int x, int y, const char *str, int fg, int bg) {}

void drawTitleBox() {}

void querySoundDriver() {
}

struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

/*
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
*/


/*
 * Code lifted (and heavily modified) from the Strife AGA port by Lantus
 * https://github.com/lantus/Strife/blob/master/i_video.c
 * */

static UWORD emptypointer[] = {
        0x0000, 0x0000,    /* reserved, must be NULL */
        0x0000, 0x0000,     /* 1 row of image data */
        0x0000, 0x0000    /* reserved, must be NULL */
};

void init() {
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

    framebuffer = (uint8_t*)calloc( 1, 128 * 128);

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
/*
#ifdef CD32
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
*/
   // SetPointer(my_window, emptypointer, 1, 16, 0, 0);

    querySoundDriver();
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
                        bufferInput = 'a';
                        break;
                    case '5':
                        bufferInput = 's';
                        break;
                    case '6':
                        bufferInput = 'd';
                        break;
                    case '8':
                        bufferInput = 'w';
                        break;
                }
        }
    } else {
        switch (rawkey) {
            case 0x4C:
                bufferInput = 'w';
                break;
            case 0x4D:
                bufferInput = 's';
                break;
            case 0x4E:
                bufferInput = 'd';
                break;
            case 0x4F:
                bufferInput = 'a';
                break;
            case 96:
            case 97:
                bufferInput = 'z';
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
                    bufferInput = 'q';
                    break;
            }
        }

        if (messageClass == VANILLAKEY && !handled) {
            switch (code) {
                case 'q':
                    bufferInput = 'q';
                    break;

                case '\n':
                case '\r':
                case 'i':
                case 'z':
                    bufferInput = 'z';
                    break;

                case 'b':
                    bufferInput = 'q';
                    break;

                case 'm':
                    bufferInput = 'e';
                    break;

                case 'h':
                    bufferInput = 'w';
                    break;

                case 's':
                    bufferInput = 'a';
                    break;
                case 'd':
                    bufferInput = 'd';
                    break;

                case 'l':
                    bufferInput = 'q';
                    break;

                case 'n':
                    bufferInput = 's';
                    break;

                case 'e':
                    break;

                case 'o':
                case 'x':
                case ' ':
                    bufferInput = 'x';
                    break;

                case 'p':
                case 'c':
                    bufferInput = 'c';
                    break;
            }
        }
    }
}

uint8_t getKey() {
    handleSystemEvents();
    uint8_t toReturn = bufferInput;
    bufferInput = '.';
    return toReturn;
}

void clear() {}


void graphicsPut(int16_t x, int16_t y) {
    framebuffer[(128 * y) + x] = 2;
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel) {
    uint8_t *ptr;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (x0 < 0 || x0 >= 128) {
        return;
    }

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 >= 128 || _y1 < 0) {
        return;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    }

    ptr = &framebuffer[(128 * _y0) + (x0)];

    for (int16_t y = _y0; y <= _y1; ++y) {
        *ptr = pixel;
        ptr += 128;
    }
}

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
int cursorPosition = 0;

char *menuItems[] = {
        "8) Use/Toggle",
        "5) Use with...",
        "9) Use/pick...",
        "6) Drop",
        "7) Next item",
        "4) Next in room",
};

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void dropItem();

void pickItem();

void clearGraphics();

void shutdownGraphics() {
    ClearPointer(my_window);
    CloseWindow(my_window);
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
}
void realPut(int x, int y, uint8_t value) {

}

void clearGraphics() {
    memset(framebuffer, 0, 128 * 128);
}

void clearScreen() {
}


void writeStrWithLimit(int _x, int y, const char *text, int limitX) {
}

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, 40);
}

void drawWindow(int tx, int ty, int tw, int th, const char *title) {}

void graphicsFlush() {
#ifdef CD32
    WriteChunkyPixels(my_window->RPort, 0, 0, 128, 128, &framebuffer[0], 128);
#else
    c2p1x1_4_c5_bm(128,128,0,0,&framebuffer[0], my_window->RPort->BitMap);
#endif
}


void showMessage(const char *message) {

}

void titleScreen() {
}

void HUD_initialPaint() {
}

void sleepForMS(uint32_t ms) {
}

void HUD_refresh() {
}
