#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Common.h"
#include "Enums.h"
#include "Core.h"
#include "Renderer.h"

#include "AY-3-8910.h"
#include "KeyboardUI.h"
#include "UI.h"
#include "font.h"

/* Sadly, I can't include conio.h - otherwise, I would get errors when building on OSX */
int kbhit(void);
int getch(void);
extern uint8_t firstFrameOnCurrentState;
enum ESoundDriver soundDriver = kAY38910;
extern enum EGameMenuState currentGameMenuState;
#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
extern uint8_t roomTransitionAnimationStep;
#endif

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

#define BUFFER_SIZEX 32
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128

#define MARGIN_TEXT_SCREEN_LIMIT 40

extern enum EDirection playerDirection;
extern int8_t cameraX;
extern int8_t cameraZ;


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

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];

void initHW(int argc, char **argv) {
    (void)argc;
    (void)argv;
    initAY38910();
    initKeyboardUI();
    needsToRedrawVisibleMeshes = 0;
    waitForKey = 0;
}

void put_sprite_8(uint16_t x, uint8_t y, uint8_t *sprite, uint8_t colour) {

    for(uint8_t c = 0; c < 8; ++c) {
        uint8_t line = *sprite;
        for (uint16_t d = 0; d < 8; ++d) {
            if (line & 1) {
                realPut( x + d, y + c, colour, NULL);
            }
            line = line >> 1;
        }
        ++sprite;
    }
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {
    size_t len = strlen(text);
    uint16_t dstX = x * 8;
    int8_t dstY = y * 8;

    uint8_t c;
    uint8_t d;
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

        for (uint8_t f = 0; f < 8; ++f) {
            uint8_t e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                realPut(dstX + (7 - e), dstY + (f), (chunk & 1), NULL);
                chunk = chunk >> 1;
            }

            fontTop++;
        }

        dstX += 8;
    }
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = (unsigned char *) 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048) + (x >> 2);
    }

    if (colour) {
        *ptr |= (8 >> (x & 3));
    } else {
        *ptr &= ~(8 >> (x & 3));
    }

    return ptr;
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {

    if (!kbhit()) {
        return kCommandNone;
    }

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                0
        );
    }


    performAction();

    uint8_t toReturn = getch();

    if (waitForKey) {
        if (toReturn == '2') {
            waitForKey = 0;
            firstFrameOnCurrentState = 1;
            needsToRedrawVisibleMeshes = 1;
            return kCommandNone;
        }

        return kCommandNone;
    }

    switch (toReturn) {
        case 30:
        case 'w':
            return kCommandUp;
        case 31:
        case 's':
            return kCommandDown;
        case 29:
        case 'q':
            return kCommandLeft;
        case 28:
        case 'e':
            return kCommandRight;
        case 'z':
            return kCommandStrafeLeft;
        case 'x':
            return kCommandStrafeRight;
        case '1':
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
    }
    return kCommandNone;
}

void clearScreen(void) {
    memFill((unsigned char *) 0xC000, 0, (320 / 4) * 200);
}

void clearGraphics(void) {
    memFill(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}

void endFrame(void) {
    if (needsToRedrawVisibleMeshes) {
        for (uint8_t y = 0; y < BUFFER_SIZEY; ++y) {
            uint8_t *line = (unsigned char *) 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048);
            memCopyToFrom(line, buffer + (y * BUFFER_SIZEX), BUFFER_SIZEX);
        }
        clearGraphics();

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
        if (roomTransitionAnimationStep) {
            return;
        }
#endif
        if (currentGameMenuState == kPlayGame) {
            put_sprite_8(
                    (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                    (cameraZ * 3) + 10,
                    &playerPositionSprite[playerDirection][0],
                    1
            );
        }
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t _y0 = y0;
    uint8_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (shouldStipple && ((_y0 & 1) == 0)) {
        _y0++;
    }

    uint8_t *ptr = &buffer[(_y0 * (BUFFER_SIZEX)) + (x0 / 4)]; /* skip to the line in pattern */
    uint8_t y;
    uint8_t pattern = (8 >> (x0 & 3));
    if (shouldStipple) {
        for (y = _y0; y <= _y1; y += 2) {
            *ptr |= pattern;
            ptr += BUFFER_SIZEX + BUFFER_SIZEX;
        }
    } else {
        for (y = _y0; y <= _y1; ++y) {
            *ptr |= pattern;
            ptr += BUFFER_SIZEX;
        }
    }
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    uint16_t x;

    for (x = 0; x < XRESMINUSONE;) {
        uint8_t y, prevY, c;
        uint8_t currByte;
        uint8_t *ptr;
        next_cluster:
        /* pixel 1 */
        y = *stencilPtr;
        prevY = y;
        ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 4)]; /* skip to the line in pattern */
        currByte = *ptr;
        currByte |= (8 >> (x & 3));

        if (x & 3) {
            *ptr = currByte;
            ++x;
            ++stencilPtr;
            continue;
        }

        for (c = 2; c < 4; ++c) {
            ++x;
            ++stencilPtr;
            y = *stencilPtr;
            if (y != prevY) {
                *ptr = currByte;
                goto next_cluster;
            }

            currByte |= (8 >> (x & 3));
        }
        *ptr = currByte;
        ++x;
        ++stencilPtr;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    buffer[(y * (BUFFER_SIZEX)) + (x / 4)] |= (8 >> (x & 3));
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    uint8_t y;
    uint16_t x;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            if (!stipple || ((x + y) & 1 )) {
                realPut(x, y, colour, NULL);
            }
        }
    }
}


void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    int16_t dx = abs(x1 - x0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t dy = abs(y1 - y0);
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = (dx > dy ? dx : -dy) >> 1;
    int16_t e2;

    for (;;) {

        if (x0 == x1 && y0 == y1) return;

        realPut(x0, y0, colour, NULL);

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
