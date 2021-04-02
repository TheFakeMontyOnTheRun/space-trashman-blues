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
#include <graphics/gfxbase.h>
#include <graphics/rastport.h>
#include <graphics/view.h>
#include <graphics/gfx.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <intuition/intuition.h>
#include <graphics/gels.h>
#include <utility/date.h>


#include <utility/date.h>
#include <clib/timer_protos.h>
#include <clib/exec_protos.h>

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
#include "Enums.h"
#include "FixP.h"
#include "Common.h"

#include "CPackedFileReader.h"

#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"

static struct IORequest timereq;
struct Library *TimerBase;

static struct timeval startTime;

void startup(){
    GetSysTime(&startTime);
}

unsigned long getMilliseconds(){
    struct timeval endTime;

    GetSysTime(&endTime);
    SubTime(&endTime, &startTime);

    return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

#define REG(xn, parm) parm __asm(#xn)
#define REGARGS __regargs

#ifdef AGA5BPP
extern void REGARGS c2p1x1_4_c5_bm(
REG(d0, UWORD chunky_x),
REG(d1, UWORD chunky_y),
REG(d2, UWORD offset_x),
REG(d3, UWORD offset_y),
REG(a0, UBYTE *chunky_buffer),
REG(a1, struct BitMap *bitmap));
#else
extern void REGARGS
c2p1x1_8_c5_bm(
REG(d0, UWORD
        chunky_x),
REG(d1, UWORD
        chunky_y),
REG(d2, UWORD
        offset_x),
REG(d3, UWORD
        offset_y),
REG(a0, UBYTE * chunky_buffer),
REG(a1, struct BitMap *bitmap)
);
#endif

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen *screen;

unsigned long t0, t1, t2;

#ifdef CD32
struct NewScreen xnewscreen = {
    0,              /* LeftEdge*/
    0,              /* TopEdge   */
    320,          /* Width     */
    200,          /* Height    */
    5,              /* Depth   */
    0,              /* DetailPen */
    1,              /* BlockPen */
    0,              /* ViewModes High-resolution, Interlaced */
    CUSTOMSCREEN,      /* Type customized screen. */
    NULL,          /* Font */
    "The Mistral Report", /* Title */
    NULL,          /* Gadget */
    NULL          /* BitMap */
};
#else

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
        (UBYTE * ) "Blank - the game",              /* Title       */
NULL,                          /* Screen      */
NULL,                          /* BitMap      */
        320,                          /* MinWidth    */
        200,                          /* MinHeight   */
        320,                          /* MaxWidth    */
        200,                          /* MaxHeight   */
        CUSTOMSCREEN                      /* Type */
};


long frame = 0;

void graphicsShutdown() {
    ClearPointer(my_window);
    CloseWindow(my_window);
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
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
    shade += ( ( ( ( ( origin & 0x0000FF )       ) << 2 ) >> 8 ) ) << 0;
    shade += ( ( ( ( ( origin & 0x00FF00 ) >> 8  ) << 2 ) >> 8 ) ) << 2;
    shade += ( ( ( ( ( origin & 0xFF0000 ) >> 16 ) << 1 ) >> 8 ) ) << 4;
    
    return shade;
}
#else

#ifdef AGA5BPP
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
#else

uint8_t getPaletteEntry(uint32_t origin) {
    uint8_t shade;

    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }

    shade = 0;
    shade += (((((origin & 0x0000FF)      ) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >>  8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}

#endif
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

void graphicsPut(int x, int y, uint8_t pixel ) {
    int offset = (320 * y) + (x << 1);
    uint8_t *ptr = &framebuffer[offset];
    *ptr = pixel;
    ++ptr;
    *ptr = pixel;
}

void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t pixel ) {

    //switching x0 with x1
    if (x0 > x1) {
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;

        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;
    }

    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

        int dx = abs(x1 - x0);
        int8_t sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;  /* error value e_xy */
        uint8_t offset;
        while (1) {
            
            offset = (320 * y0) + (2 * x0);
            framebuffer[offset] = pixel;
            framebuffer[offset + 1] = pixel;
            
            /* loop */
            if (x0 == x1 && y0 == y1) return;
            int e2 = 2 * err;

            if (e2 >= dy) {
                err += dy; /* e_xy+e_x > 0 */
                x0 += sx;
            }

            if (e2 <= dx) {
                /* e_xy+e_y < 0 */
                err += dx;
                y0 += sy;
            }
        }
    }
}

void graphicsHorizontalLine(int16_t x0, int16_t x1, int16_t y, uint8_t pixel) {
    int offset;
    uint8_t *ptr;
    int16_t _x0 = x0;
    int16_t _x1 = x1;

    
    
    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }
    
    if (_x0 >= 128 || _x1 < 0 ) {
        return;
    }
    
    if ( _x0 < 0 ) {
        _x0 = 0;
    }
    
    if ( _x1 >= 128 ) {
        _x0 = 127;
    }
    
    offset = (320 * y) + (_x0 << 1);
    ptr = &framebuffer[offset];
    
    memset( ptr, pixel, (_x1 - _x0 ) * 2 );
}

void graphicsVerticalLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel ) {
    int offset;
    uint8_t *ptr;
    int16_t _y0 = y0;
    int16_t _y1 = y1;
    
    if (x0 < 0 || x0 >= 256 ) {
        return;
    }
    
    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }
    
    if ( _y0 >= 128 || _y1 < 0 ) {
        return;
    }
    
    if ( _y0 < 0) {
        _y0 = 0;
    }
    
    if ( _y1 >= 128) {
        _y1 = 127;
    }
    
    offset = (320 * _y0) + (x0 << 1);
    ptr = &framebuffer[offset];
    
    for (int16_t y = _y0; y <= _y1; ++y) {
        *ptr = pixel;
        ++ptr;
        *ptr = pixel;
        ptr += 319;
    }
}


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
#ifdef AGA5BPP

    for (r = 0; r < 256; r += 255) {
        for (g = 0; g < 256; g += 255) {
            for (b = 0; b < 256; b += 255) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palete[paletteEntry].r = r;
                palete[paletteEntry].g = g;
                palete[paletteEntry].b = b;
            }
        }
    }

    for (c = 0; c < 16; ++c) {
        SetRGB4(&screen->ViewPort, c, palete[c].r, palete[c].g,
                palete[c].b);
    }
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
#endif

    SetPointer(my_window, emptypointer, 1, 16, 0, 0);

    defaultFont = loadBitmap("font.img");

    OpenDevice("timer.device", 0, &timereq, 0);
    TimerBase = timereq.io_Device;
    startup();
    for ( int c = 0; c < 10; ++c) {
        OwnBlitter();
        c2p1x1_4_c5_bm(320, dirtyLineY1 - dirtyLineY0, 0, dirtyLineY0, &framebuffer[dirtyLineY0 * 320],
                       my_window->RPort->BitMap);
        DisownBlitter();
    }
    t0 = getMilliseconds();

    startup();
    OwnBlitter();
    for ( int c = 0; c < 10; ++c ) {
        c2p1x1_4_c5_bm(320, dirtyLineY1 - dirtyLineY0, 0, dirtyLineY0, &framebuffer[dirtyLineY0 * 320],
                       my_window->RPort->BitMap);
    }
    DisownBlitter();
    t1 = getMilliseconds();



    OwnBlitter();
    startup();
    for ( int c = 0; c < 10; ++c ) {
        c2p1x1_4_c5_bm(320, dirtyLineY1 - dirtyLineY0, 0, dirtyLineY0, &framebuffer[dirtyLineY0 * 320],
                       my_window->RPort->BitMap);
    }
    t2 = getMilliseconds();
    DisownBlitter();

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
                        mBufferedCommand = kCommandStrafeLeft;
                        break;
                    case '5':
                        mBufferedCommand = kCommandDown;
                        break;
                    case '6':
                        mBufferedCommand = kCommandStrafeRight;
                        break;
                    case '8':
                        mBufferedCommand = kCommandUp;
                        break;
                }
        }
    } else {
        switch (rawkey) {
            case 0x4C:
                mBufferedCommand = kCommandUp;
                break;
            case 0x4D:
                mBufferedCommand = kCommandDown;
                break;
            case 0x4E:
                mBufferedCommand = kCommandStrafeRight;
                break;
            case 0x4F:
                mBufferedCommand = kCommandStrafeLeft;
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
                    mBufferedCommand = kCommandStrafeLeft;
                    break;

                case 'm':
                    mBufferedCommand = kCommandStrafeRight;
                    break;

                case 'h':
                    mBufferedCommand = kCommandUp;
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
#ifndef CD32
#ifdef AGA8BPP
    OwnBlitter();
    c2p1x1_8_c5_bm(320, dirtyLineY1 - dirtyLineY0, 0, dirtyLineY0, &framebuffer[dirtyLineY0 * 320], my_window->RPort->BitMap);
    DisownBlitter();
#else
    OwnBlitter();
    c2p1x1_4_c5_bm(320, dirtyLineY1 - dirtyLineY0, 0, dirtyLineY0, &framebuffer[dirtyLineY0 * 320], my_window->RPort->BitMap);
    DisownBlitter();
#endif
#else
    WriteChunkyPixels(my_window->RPort, 0, 0, 320, 200, &framebuffer[0], 320);
#endif
}
