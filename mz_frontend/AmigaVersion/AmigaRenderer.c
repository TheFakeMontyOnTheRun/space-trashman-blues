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

#include <stdio.h>
#include <stdlib.h>


#define TRANSPARENCY_COLOR 17

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[320 * 200];

void graphicsFlush();


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
extern void REGARGS c2p1x1_8_c5_bm(
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

long frame = 0;


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
        (UBYTE * ) "The Mistral Report",              /* Title       */
NULL,                          /* Screen      */
NULL,                          /* BitMap      */
        320,                          /* MinWidth    */
        200,                          /* MinHeight   */
        320,                          /* MaxWidth    */
        200,                          /* MaxHeight   */
        CUSTOMSCREEN                      /* Type */
};


struct RGB8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};


/*
 * Code lifted (and heavily modified) from the Strife AGA port by Lantus
 * https://github.com/lantus/Strife/blob/master/i_video.c
 * */

static UWORD emptypointer[] = {
        0x0000, 0x0000,    /* reserved, must be NULL */
        0x0000, 0x0000,     /* 1 row of image data */
        0x0000, 0x0000    /* reserved, must be NULL */
};

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
                        mBufferedCommand = 'a';
                        break;
                    case '5':
                        mBufferedCommand = 's';
                        break;
                    case '6':
                        mBufferedCommand = 'd';
                        break;
                    case '8':
                        mBufferedCommand = 'w';
                        break;
                }
        }
    } else {
        switch (rawkey) {
            case 0x4C:
                mBufferedCommand = 'w';
                break;
            case 0x4D:
                mBufferedCommand = 's';
                break;
            case 0x4E:
                mBufferedCommand = 'd';
                break;
            case 0x4F:
                mBufferedCommand = 'a';
                break;
            case 96:
            case 97:
/*				mBufferedCommand = kCommandFire3; */
                break;
        }
    }
}




void shutdownGraphics() {
    ClearPointer(my_window);
    CloseWindow(my_window);
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
}


void clearGraphics() {
    memset(framebuffer, 0, 320 * 200);
}

void writeStr(int nColumn, int nLine, char *str, int fg, int bg) {

}

uint8_t getKey() {
    struct IntuiMessage *my_message;
    ULONG messageClass;
    USHORT code;

    mBufferedCommand = '.';

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
                    mBufferedCommand = 'q';
                    break;
            }
        }

        if (messageClass == VANILLAKEY && !handled) {
            switch (code) {
                case 'q':
                    mBufferedCommand = 'q';
                    break;

                case '\n':
                case '\r':
                case 'i':
                case 'z':
                    /*mBufferedCommand = kCommandFire1;*/
                    break;

                case 'b':
                    mBufferedCommand = 'a';
                    break;

                case 'm':
                    mBufferedCommand = 'd';
                    break;

                case 'h':
                    mBufferedCommand = 'w';
                    break;

                case 's':
                    /*mBufferedCommand = kCommandStrafeLeft;*/
                    break;
                case 'd':
                    /*mBufferedCommand = kCommandStrafeRight;*/
                    break;

                case 'l':
                    mBufferedCommand = 'q';
                    break;

                case 'n':
                    mBufferedCommand = 's';
                    break;

                case 'e':
                    break;

                case 'o':
                case 'x':
                case ' ':
                    /* mBufferedCommand = kCommandFire2; */
                    break;

                case 'p':
                case 'c':
                    /* mBufferedCommand = kCommandFire3; */
                    break;
            }
        }
    }

    return mBufferedCommand;
}

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
    /*
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
     */
#else

#endif

    SetPointer(my_window, emptypointer, 1, 16, 0, 0);
}


void flipRenderer() {

}

void graphicsPut(int x, int y ) {
    x = x * 2;
    framebuffer[(320 * y) + x] = 1;
    framebuffer[(320 * y) + x + 1] = 1;
}


void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t colour) {

    if (x0 == x1) {

        int16_t _y0 = y0;
        int16_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        for (int16_t y = _y0; y <= _y1; ++y) {
            if (x0 < 0 || x0 >= 256 || y < 0 || y >= 128) {
            //    continue;
            }

            graphicsPut(x0, y );
        }
        return;
    }

    if (y0 == y1) {
        int16_t _x0 = x0;
        int16_t _x1 = x1;

        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }

        for (int16_t x = _x0; x <= _x1; ++x) {
            if (x < 0 || x >= 256 || y0 < 0 || y0 >= 128) {
             //   continue;
            }

            graphicsPut(x, y0 );
        }
        return;
    }

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
        int sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0);
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;  /* error value e_xy */

        while (1) {
            framebuffer[(320 * y0) + (2 * x0)] = colour;
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

void hLine(int16_t x0, int16_t x1, int16_t y, uint8_t colour) {
    fix_line(x0, y, x1, y, colour);
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint8_t colour) {
    fix_line(x0, y0, x0, y1, colour);
}



void fill(
        const int16_t x,
        const int16_t y,
        const int16_t dx,
        const int16_t dy,
        const uint8_t pixel,
        const int stipple) {

    uint8_t *destination = &framebuffer[0];
    int16_t py;

    for (py = 0; py < dy; ++py) {
        uint8_t *destinationLineStart = destination + (320 * (y + py)) + x;

        if (!stipple) {
            memset (destinationLineStart, pixel, dx);
        } else {
            int16_t px;
            for (px = 0; px < dx; ++px) {

                destinationLineStart++;

                if ((px + py) & 1) {
                    //   *destinationLineStart = pixel;
                }
            }
        }


    }
}


void graphicsFill(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t colour) {
    fill( x0, y0, x1 - x0, y1 - y0, colour, 0 );
}


void graphicsFlush() {
#ifdef AGA8BPP
    OwnBlitter();
    WaitBlit();
    c2p1x1_8_c5_bm(320,200,0,0,&framebuffer[0], my_window->RPort->BitMap);
    DisownBlitter();
#else
    OwnBlitter();
    WaitBlit();
    c2p1x1_4_c5_bm(320, 200, 0, 0, &framebuffer[0], my_window->RPort->BitMap);
    DisownBlitter();
#endif
}