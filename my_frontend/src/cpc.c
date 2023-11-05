#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <games.h>
#include <psg.h>
#include <sound.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"

#include "AY-3-8910.h"
#include "KeyboardUI.h"
#include "UI.h"
#include "font.h"

/*  Required since we have our own memory allocator abstraction */
uint16_t heap = 0;

enum ESoundDriver soundDriver = kNoSound;

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

uint8_t updateDirection;

#define BUFFER_SIZEX 32
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128

#define MARGIN_TEXT_SCREEN_LIMIT 40

int getch(void);

uint16_t lineStart[128];
uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];

void initHW(void) {
    initAY38910();
    initKeyboardUI();
    updateDirection = 1;
    for (int y = 0; y < 128; ++y) {
        lineStart[y] = 0xC000 + ((y >> 3) * 80) + ((y & 7) * 2048);
    }
}

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {

    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t x = _x;

    uint8_t *lineBase = (unsigned char *) 0xC000 + ((((y * 8)) / 8) * 80) + ((((y * 8)) & 7) * 2048);

    for (; c < len && y < 64; ++c) {

        char cha = *ptr;

        if (x == limitX) {
            ++y;
            lineBase += 2048;
            x = _x;
        } else if (cha == '\n') {
            ++y;
            lineBase += 2048;
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

        uint8_t *fontTop = &font[((cha - 32) << 3)];

        uint8_t *line = lineBase + 2 * x + 1;

        for (int d = 0; d < 8; ++d) {
            int e;
            uint8_t chunk = *fontTop;
            uint8_t *pixel = line;

            *pixel = 0;

            for (e = 0; e < 4; ++e) {
                if (chunk & 1) {
                    *pixel |= (16 << e);
                }
                chunk = chunk >> 1;
            }

            --pixel;

            *pixel = 0;

            for (e = 0; e < 4; ++e) {
                if (chunk & 1) {
                    *pixel |= (16 << e);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
            line += 2048;
        }

        ++x;
        ++ptr;
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

void clearTextScreen(void) {
  int c, d;
  for (c = 16; c < 24; ++c ) {
    for (d = 1; d < 32; ++d ){
      writeStrWithLimit(d, c, " ", 256 / 8, 2, 0);
    }
  }
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
    uint8_t input = getch();

    performAction();

    switch (input) {
        case 30:
            return 'w';
        case 31:
            return 's';
        case 29:
            updateDirection = 1;
            return 'q';
        case 28:
            updateDirection = 1;
            return 'e';
        case 'z':
            return 'a';
        case 'x':
            return 'd';
        case 'm':
            drawMap();
            return '.';
    }
    return input;
}

void shutdownGraphics(void) {
}

void clearScreen(void) {
    memset((unsigned char *) 0xC000, 0, (320 / 4) * 200);
}

void clearGraphics(void) {
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void graphicsFlush(void) {

    for (int y = 0; y < BUFFER_SIZEY; ++y) {
        uint8_t *line = (unsigned char *) lineStart[y];
        memcpy(line, buffer + (y * BUFFER_SIZEX), BUFFER_SIZEX);
    }

    if (updateDirection) {
        char direction[8] = {'N', 0, 'E', 0, 'S', 0, 'W', 0};
        updateDirection = 0;
        writeStrWithLimit(12, 18, &direction[getPlayerDirection() * 2], 31, 2, 0);
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

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {

    if (ptr == NULL) {
        ptr = &buffer[(y * (BUFFER_SIZEX)) + (x / 4)]; /* skip to the line in pattern */
    }

    *ptr |= (8 >> (x & 3));

    return ptr;
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    int x;

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

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour, NULL);
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
