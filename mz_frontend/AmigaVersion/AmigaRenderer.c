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

#define NORMALIZE(x) (((x * 16) / 256))

extern void REGARGS
c2p1x1_4_c5_bm(
REG(d0, UWORD chunky_x),
REG(d1, UWORD chunky_y),
REG(d2, UWORD offset_x),
REG(d3, UWORD offset_y),
REG(a0, UBYTE * chunky_buffer),
REG(a1, struct BitMap *bitmap)
);

struct IntuitionBase *IntuitionBase;
struct GfxBase *GfxBase;
extern struct ExecBase *SysBase;

struct Window *my_window;
struct Screen *screen;

uint8_t *framebuffer;
uint8_t bufferInput = '.';

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

    framebuffer = (uint8_t *) calloc(1, 128 * 128);

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

    SetPointer(my_window, emptypointer, 1, 16, 0, 0);

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


void graphicsPut(int16_t x, int16_t y, uint8_t colour) {
    if (colour >= 16) {
        if ((x + y) & 1) {
            framebuffer[(128 * y) + x] = 0;
        } else {
            framebuffer[(128 * y) + x] = colour - 16;
        }
    } else {
        framebuffer[(128 * y) + x] = colour;
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint8_t colour) {
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

    if (colour <= 16) {
        for (int16_t y = _y0; y <= _y1; ++y) {
            *ptr = colour;
            ptr += 128;
        }
    } else {
        colour = colour - 16;
        uint8_t stipple = ((x0 + y0) & 1);

        for (int16_t y = _y0; y <= _y1; ++y) {
            stipple = !stipple;

            if (stipple) {
                *ptr = colour;
            } else {
                *ptr = 0;
            }

            ptr += 128;
        }
    }
}

void hLine(int16_t x0, int16_t x1, int16_t y, uint8_t colour) {
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

    if (_x1 >= 128) {
        _x1 = 127;
    }

    if (colour <= 16) {
        for (int x = _x0; x <= _x1; ++x) {
            framebuffer[(128 * y) + x] = colour;
        }
    } else {
        colour = colour - 16;
        uint8_t stipple = ((x0 + y) & 1);

        for (int x = _x0; x <= _x1; ++x) {
            stipple = !stipple;

            if (stipple) {
                framebuffer[(128 * y) + x] = colour;
            } else {
                framebuffer[(128 * y) + x] = 0;
            }
        }
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
    c2p1x1_4_c5_bm(128, 128, 0, 0, &framebuffer[0], my_window->RPort->BitMap);
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