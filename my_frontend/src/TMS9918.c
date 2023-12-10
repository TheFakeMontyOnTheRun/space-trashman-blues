/*
   Created by Daniel Monteiro on 11/07/2023.
*/

#include <stdint.h>

#include <msx/gfx.h>

#include "font.h"

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];

void setColour(uint8_t colour) {
    set_color(colour, 0, 0);
}

void flush3DBuffer(void) {
    uint8_t *ptr = &buffer[0];

    for (uint8_t y = 0; y < (BUFFER_RESY); y += 8) {
        /* 248 = ~7 */
        vwrite(ptr, (y << 5), 16 * 8);
        ptr += 8 * 16;
    }

    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
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

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    if (ptr == NULL) {
        ptr = &buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)];
    }

    *ptr |= (128 >> (x & 7));

    return ptr;
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    uint16_t x;

    for (x = 0; x < (BUFFER_RESX - 1);) {
        uint8_t y, prevY, c;
        uint8_t *ptr;
        uint8_t currByte;
        next_cluster:

        y = *stencilPtr;
        prevY = y;
        ptr = &buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)];
        currByte = *ptr;
        currByte |= (128 >> (x & 7));

        if (x & 7) {
            *ptr = currByte;
            ++x;
            ++stencilPtr;
            continue;
        }

        for (c = 2; c < 8; ++c) {
            ++x;
            ++stencilPtr;
            y = *stencilPtr;
            if (y != prevY) {
                *ptr = currByte;
                goto next_cluster;
            }
            currByte |= (128 >> (x & 7));
        }
        *ptr = currByte;

        ++x;
        ++stencilPtr;
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

void clearGraphics(void) {
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void shutdownGraphics(void) {
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    setColour(colour);
    plot(x, y);
    return NULL;
}

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX) {

    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t chary = 0;
    uint8_t x = _x;
    char lastChar = 0xFF;
    uint8_t *fontTop;

    for (; c < len && y < 64; ++c) {

        char cha = *ptr;

        if (x == limitX) {
            ++y;
            x = _x;
        } else if (cha == '\n') {
            ++y;
            x = _x;
            ++ptr;
            continue;
        }

        if (cha >= 'a') {
            if (cha <= 'z') {
                cha = (cha - 'a') + 'A';
            } else {
                cha -= ('z' - 'a');
            }
        }

        if (cha != lastChar) {
            fontTop = &font[((cha - 32) << 3)];
            lastChar = cha;
        }
        vwrite(fontTop, map_pixel(x << 3, y << 3), 8);
        ++x;
        ++ptr;
    }
}

void clearTextScreen(void) {
    uint8_t c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 1; d < 32; ++d) {
            writeStrWithLimit(d, c, " ", 256 / 8);
        }
    }
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    uint8_t x, y;
    setColour(colour);
    if (stipple) {
        for (y = y0; y < y1; ++y) {
            for (x = x0 + (y & 1); x < x1; x += 2) {
                plot(x, y);
            }
        }
    } else {
        for (y = y0; y < y1; ++y) {
            for (x = x0; x < x1; ++x) {
                plot(x, y);
            }
        }
    }
}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    setColour(colour);
    draw(x0, y0, x1, y1);
}
