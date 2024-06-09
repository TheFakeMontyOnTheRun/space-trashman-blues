#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include <osbind.h>
#include <mint/sysbind.h>
#include <mint/osbind.h>

#include "AtariInt.h"
#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"
#include "font.h"

uint16_t *physBase;
uint16_t *logBase;

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

uint8_t *framebuffer;
uint8_t bufferInput = '.';
extern uint8_t firstFrameOnCurrentState;

#define NORMALIZE(x) (((x * 8) / 256))

void framebuffer_set_palette_entry(int index, int red, int green, int blue) {
    *(uint16_t *) (0xffff8240 + (index * 2)) = blue | (green << 4) | (red << 8);
}

void initHW(int argc, char** argv) {
    framebuffer = (uint8_t *) calloc(1, 256 * 160);

    physBase = Physbase();
    logBase = Logbase();
    memset(logBase, 0, 32000);
    memset(physBase, 0, 32000);
    Setscreen(-1, -1, 0);

    framebuffer_set_palette_entry(0, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0x00));
    framebuffer_set_palette_entry(1, NORMALIZE(0x00), NORMALIZE(0x00), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(2, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0x00));
    framebuffer_set_palette_entry(3, NORMALIZE(0x00), NORMALIZE(0xAA), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(4, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0x00));
    framebuffer_set_palette_entry(5, NORMALIZE(0xAA), NORMALIZE(0x00), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(6, NORMALIZE(0xAA), NORMALIZE(0x55), NORMALIZE(0x00));
    framebuffer_set_palette_entry(7, NORMALIZE(0xAA), NORMALIZE(0xAA), NORMALIZE(0xAA));
    framebuffer_set_palette_entry(8, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0x55));
    framebuffer_set_palette_entry(9, NORMALIZE(0x55), NORMALIZE(0x55), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(10, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0x55));
    framebuffer_set_palette_entry(11, NORMALIZE(0x55), NORMALIZE(0xFF), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(12, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0x55));
    framebuffer_set_palette_entry(13, NORMALIZE(0xFF), NORMALIZE(0x55), NORMALIZE(0xFF));
    framebuffer_set_palette_entry(14, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0x55));
    framebuffer_set_palette_entry(15, NORMALIZE(0xFF), NORMALIZE(0xFF), NORMALIZE(0xFF));
}

void handleSystemEvents(void) {
    bufferInput = Cnecin();
}

void endFrame(void) {

    uint8_t *index = &framebuffer[0];
    uint16_t lineOffset = 0;
    uint16_t *words = (uint16_t *) logBase;

    for (uint16_t y = 160; y; y--) {
        memset(logBase + lineOffset, 0, 80);
        for (uint16_t x = 0; x < 256; ++x) {

            uint8_t value = *index++;
            uint16_t offset = lineOffset + ((x >> 4) << 2);
            uint16_t bitPattern = (1 << (15 - (x & 15)));
            uint16_t *ptr = &words[offset];

            if (value & 1) {
                *ptr |= bitPattern;
            }

            ptr++;

            if (value & 2) {
                *ptr |= bitPattern;
            }

            ptr++;

            if (value & 4) {
                *ptr |= bitPattern;
            }

            ptr++;

            if (value & 8) {
                *ptr |= bitPattern;
            }
        }
        lineOffset += 80;
    }

    uint16_t *tmp;
    tmp = physBase;
    physBase = logBase;
    logBase = tmp;
    Setscreen(logBase, physBase, -1);
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

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    framebuffer[(256 * y) + x] = colour;
}

void realPut(int x, int y, uint8_t colour) {
    framebuffer[(256 * y) + x] = colour;
}

void clearTextScreen(void) {
    int c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 0; d < 32; ++d) {
            drawTextAtWithMarginWithFiltering(d, c, 256, " ", 2, ' ');
        }
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t pixel) {
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
        *ptr = pixel;
        ptr += 256;
    }
}

void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0 || y >= 128) {
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

void clearScreen(void) {
    memset(framebuffer, 0, 256 * 160);
}

void clearGraphics(void) {
    memset(framebuffer, 0, 256 * 160);
}

void startFrame(int x, int y, int width, int height) {

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
