#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"
#include "font.h"

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;

uint8_t *framebuffer;
uint8_t bufferInput = '.';
extern uint8_t firstFrameOnCurrentState;

unsigned char imageBuffer[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

void shutdownGraphics(void) {
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {

    if (x0 < 0) {
        return;
    }

    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    }

    for (y = _y0; y <= _y1; ++y) {
        imageBuffer[(256 * y) + x0] = colour;
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

    for (int x = _x0; x <= _x1; ++x) {
        imageBuffer[(256 * y) + x] = colour;
    }
}

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    if (x < 0) {
        x = 0;
    }

    if (x >= 128) {
        x = 127;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= 128) {
        y = 127;
    }

    imageBuffer[(256 * y) + x] = colour;
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

    imageBuffer[(256 * y) + x] = value;

}

void clearGraphics(void) {
    memset(imageBuffer, 0, 256 * 160);
}

void initHW(int argc, char** argv) {
    clearScreen();
}

void clearScreen(void) {
    asm volatile("movb $0x0, %%ah\n\t"
                 "movb $0x0D, %%al\n\t"
                 "int $0x10\n\t"
            :
            :
            : "ax"
            );
}

void handleSystemEvents(void) {
    unsigned char toReturn = 255;


    asm volatile ("movb $0x00, %%ah\n\t"
                  "movb $0x00, %%al\n\t"
                  "int $0x16       \n\t"
                  "movb %%al, %0\n\t"
            : "=rm"(toReturn)
            :
            : "ax"
            );

    asm volatile("movb $0x0C, %%ah\n\t"
                 "movb $0x00, %%al\n\t"
                 "int $0x21\n\t"
            :
            :
            : "ax"
            );

    bufferInput = toReturn;
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

void clearTextScreen(void) {
    int c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 0; d < 32; ++d) {
            drawTextAtWithMarginWithFiltering(d, c, 256, " ", 2, ' ');
        }
    }
}

void startFrame(int x, int y, int width, int height) {

}

void endFrame(void) {

    uint8_t *bufferPtr = &imageBuffer[0];

    for (int y = 0; y < 160; ++y) {
        for (int x = 0; x < 256; ++x) {
            uint16_t index = *bufferPtr;

            int pixel = index;
            int px = x;
            int py = y;

            asm volatile ("movb $0x0C, %%ah\n\t"
                          "movb %0,    %%al\n\t"
                          "movb $0x0,  %%bh\n\t"
                          "movw %1,    %%cx\n\t"
                          "movw %2,    %%dx\n\t"
                          "int $0x10\n\t"
                    :
                    :"rm" (pixel), "rm" (px), "rm" (py)
                    : "ax", "bx", "cx", "dx"
                    );
            bufferPtr++;
        }
    }
}
