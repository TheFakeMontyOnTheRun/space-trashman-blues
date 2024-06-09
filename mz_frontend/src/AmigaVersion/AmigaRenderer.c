#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

#include <intuition/intuition.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <clib/graphics_protos.h>
#include <proto/keymap.h>

#include "AmigaInt.h"

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"
#include "font.h"

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

extern uint8_t firstFrameOnCurrentState;
extern enum EGameMenuState currentGameMenuState;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
extern uint8_t roomTransitionAnimationStep;
#endif

extern enum EDirection playerDirection;
extern int8_t cameraX;
extern int8_t cameraZ;

struct NewScreen xnewscreen = {
        0,              /* LeftEdge*/
        0,              /* TopEdge   */
        320,          /* Width     */
        200,          /* Height    */
        4,              /* Depth   */
        0,              /* DetailPen */
        1,              /* BlockPen */
        0,              /* ViewModes High-resolution, Interlaced */
        CUSTOMSCREEN,      /* Type customized screen. */
        NULL,          /* Font */
        "Sub Mare Imperium: Derelict", /* Title */
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
        (UBYTE * ) "Sub Mare Imperium: Derelict",              /* Title       */
        NULL,                          /* Screen      */
        NULL,                          /* BitMap      */
        320,                          /* MinWidth    */
        200,                          /* MinHeight   */
        320,                          /* MaxWidth    */
        200,                          /* MaxHeight   */
        CUSTOMSCREEN                      /* Type */
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
void handleSystemEvents(void) {

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

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                    bufferInput = code;
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



void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    framebuffer[(256 * y) + x] = colour;
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {
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

    ptr = &framebuffer[(256 * _y0) + (x0)];

    for (int16_t y = _y0; y <= _y1; ++y) {
        *ptr = colour;
        ptr += 256;
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

    if (_x1 >= 128) {
        _x1 = 127;
    }

    uint8_t *ptr = &framebuffer[(256 * y) + _x0];
    for (int16_t x = _x0; x <= _x1; ++x) {
        *ptr++ = colour;
    }
}

void shutdownGraphics(void) {
    ClearPointer(my_window);
    CloseWindow(my_window);
    CloseScreen(screen);
    CloseLibrary((struct Library *) IntuitionBase);
}

void realPut(int x, int y, uint8_t value) {
    if (x < 0) {
        x = 0;
    }

    if (x >= 256) {
        x = 255;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= 160) {
        y = 159;
    }

    framebuffer[(256 * y) + x] = value;
}

void clearGraphics(void) {
    memset(framebuffer, 0, 256 * 160);
}

void clearScreen(void) {
}

void graphicsFlush(void) {
    c2p1x1_4_c5_bm(256, 160, 0, 0, &framebuffer[0], my_window->RPort->BitMap);
}

char playerPositionSprite[4][8]={
        {
                0b00011000,
                0b00111100,
                0b01111110,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000
        },
        {
                0b00100000,
                0b01100000,
                0b11100000,
                0b11100000,
                0b01100000,
                0b00100000,
                0b00000000,
                0b00000000
        },
        {
                0b01111110,
                0b00111100,
                0b00011000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000
        },
        {
                0b00000100,
                0b00000110,
                0b00000111,
                0b00000111,
                0b00000110,
                0b00000100,
                0b00000000,
                0b00000000
        },
};

void put_sprite_8(uint16_t x, uint8_t y, uint8_t *sprite, uint8_t colour) {

    for(uint8_t c = 0; c < 8; ++c) {
        uint8_t line = *sprite;
        for (uint16_t d = 0; d < 8; ++d) {
            if (line & 1) {
                realPut( x + d, y + c, colour);
            }
            line = line >> 1;
        }
        ++sprite;
    }
}

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptrToByte) {

}

void graphicsPutPointArray(uint8_t *y128Values) {

}


void initHW(int argc, char **argv) {

    framebuffer = (uint8_t *) calloc(1, 256 * 160);

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

    initKeyboardUI();
    clearGraphics();
}

enum ECommand getInput(void) {


    switch(bufferInput) {
        case 'q':
            return kCommandLeft;
        case 'w':
            return kCommandUp;
        case 's':
            return kCommandDown;
        case 'e':
            return kCommandRight;
        case 'a':
            return kCommandStrafeLeft;
        case 'd':
            return kCommandStrafeRight;
        case 'l':
            return kCommandBack;

        case '1':
            if (waitForKey) {
                waitForKey = 0;
                firstFrameOnCurrentState = 1;
                needsToRedrawVisibleMeshes = 1;
                return kCommandNone;
            }

            return kCommandFire1;
        case '2':
            return kCommandFire2;
        case '3':
            return kCommandFire3;
        case '4':
            return kCommandFire4;
        case '5':
            return kCommandFire5;
        case '6':
            return kCommandFire6;
        case 'k':
            exit(0);
    }

    return kCommandNone;
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {

    size_t len = strlen(text);
    int32_t dstX = x * 8;
    int32_t dstY = y * 8;

    size_t c;
    size_t d;
    uint8_t lastSpacePos = 0xFF;

    for (c = 0; c < len; ++c) {

        char currentChar = text[c];

        if (currentChar == '-') {
            currentChar = charToReplaceHifenWith;
        }

        if (currentChar == '\n' || dstX >= (margin)) {
            dstX = x * 8;
            dstY += 8;
            continue;
        }

        if (dstY >= YRES_FRAMEBUFFER) {
            return;
        }

        if (currentChar == ' ') {
            lastSpacePos = c;
        } else {
            if ((c - 1) == lastSpacePos) {
                d = c;
                while (d < len && text[d] != ' ') ++d;

                if ((dstX + ((d - c ) * 8)) >= margin ) {
                    dstX = x * 8;
                    dstY += 8;
                }
            }
        }


        if (currentChar >= 'a') {
            if (currentChar <= 'z') {
                currentChar = (currentChar - 'a') + 'A';
            } else {
                currentChar -= ('z' - 'a');
            }
        }

        uint8_t *fontTop = &font[((currentChar - 32) << 3)];

        for (int f = 0; f < 8; ++f) {
            int e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    realPut(dstX + (7 - e), dstY + (f), 1);
                } else {
                    realPut(dstX + (7 - e), dstY + (f), 0);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
        }
        dstX += 8;
    }
}


void startFrame(int x, int y, int width, int height) {

}

void endFrame(void) {

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                1
        );
    }
    graphicsFlush();
}


void clearTextScreen(void) {
    int c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 0; d < 32; ++d) {
            drawTextAtWithMarginWithFiltering(d, c, 256, " ", 2, ' ');
        }
    }
}

void enterTextMode(void) {}

void exitTextMode(void) {
    clearScreen();
}


void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour);
        }
    }
}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) >> 1;
    int e2;
    for (;;) {

        if (x0 == x1 && y0 == y1) break;

        realPut(x0, y0, colour);

        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}
