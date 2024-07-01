/*
   Created by Daniel Monteiro on 11/07/2023.
*/

#include <stdint.h>

#include <msx/gfx.h>

#include "Common.h"
#include "font.h"
#include "Enums.h"

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
                0b00000100,
                0b00000110,
                0b00000111,
                0b00000111,
                0b00000110,
                0b00000100,
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
                0b00100000,
                0b01100000,
                0b11100000,
                0b11100000,
                0b01100000,
                0b00100000,
                0b00000000,
                0b00000000
        },
};

extern int8_t cameraX;
extern int8_t cameraZ;
extern enum EDirection playerDirection;
extern enum EGameMenuState currentGameMenuState;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
extern uint8_t roomTransitionAnimationStep;
#endif

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];

void setColour(uint8_t colour) {
    (void) colour;
}

void clearGraphics(void) {
    memFill(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void flush3DBuffer(void) {
    uint8_t *ptr = buffer;

    for (uint8_t y = 0; y < BUFFER_RESY; y += 8) {
        vwrite(ptr, y << 5, 16 * 8);
        ptr += 128; /* 8 * 16 */
    }

    for (uint8_t i = 0; i < 4; ++i) {
        vdp_set_sprite_8(i, playerPositionSprite[i]);
    }

    clearGraphics();

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
    if (roomTransitionAnimationStep) {
        return;
    }
#endif

    if (currentGameMenuState == kPlayGame) {
        vdp_put_sprite_8(
                0,
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                playerDirection,
                15
        );
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
        ++_y0;
    }

    uint8_t patternLine = (_y0 & 7); /* which line inside the pattern; */
    uint8_t *ptr = &buffer[((_y0 & ~7) << 4) + (x0 & ~7) + patternLine];
    uint8_t shiftXAnd7 = 128 >> (x0 & 7);

    if (shouldStipple) {
        for (uint8_t y = _y0; y <= _y1; y += 2) {
            *ptr |= shiftXAnd7;
            patternLine += 2;
            ptr += 2;

            if (patternLine >= 8) {
                patternLine = 0;
                ptr += (16 * 8) - 8;
            }
        }
    } else {
        for (uint8_t y = _y0; y <= _y1; ++y) {
            *ptr |= shiftXAnd7;
            ++patternLine;
            ++ptr;

            if (patternLine >= 8) {
                patternLine = 0;
                ptr += (16 * 8) - 8;
            }
        }
    }
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    uint16_t x = 0;

    while (x < (BUFFER_RESX - 1)) {
        uint8_t y = *stencilPtr;
        uint8_t prevY = y;
        uint8_t *ptr = &buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)];
        uint8_t currByte = *ptr;

        while (1) {
            currByte |= (128 >> (x & 7));

            if (x & 7) {
                *ptr = currByte;
                ++x;
                ++stencilPtr;
                break;
            }

            for (uint8_t c = 2; c < 8; ++c) {
                ++x;
                ++stencilPtr;
                y = *stencilPtr;
                if (y != prevY) {
                    *ptr = currByte;
                    goto next_point;
                }
                currByte |= (128 >> (x & 7));
            }

            *ptr = currByte;
            ++x;
            ++stencilPtr;
            break;
        }

        next_point:
        continue;
    }
}


void graphicsPut(uint8_t x, uint8_t y) {
    buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)] |= (128 >> (x & 7));
}

void clearScreen(void) {
    set_mode(mode_2);
    fill(MODE2_ATTR, 0xF1, MODE2_MAX);
}

void initTMS9918(void) {
    clearScreen();
}

#ifdef EMIT_QUIT_OPTION
void shutdownGraphics(void) {
}
#endif


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

        vwrite(fontTop, map_pixel(dstX, dstY), 8);
        dstX += 8;
    }
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    uint8_t x, y;

    for (y = y0; y < y1; ++y) {
        for (x = x0 + ((y & 1) && stipple); x < x1; x += ( stipple ? 2 : 1)) {
            plot(x, y);
        }
    }
}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    draw(x0, y0, x1, y1);
}
