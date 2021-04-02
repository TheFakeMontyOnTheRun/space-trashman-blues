#include <conio.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/nearptr.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "Enums.h"
#include "FixP.h"
#include "Common.h"
#include "CPackedFileReader.h"

#include "Engine.h"
#include "LoadBitmap.h"
#include "CRenderer.h"

enum EColor {
    COLOR_BLACK,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_CYAN,
    COLOR_RED,
    COLOR_MAGENTA,
    COLOR_BROWN,
    COLOR_LIGHT_GRAY,
    COLOR_DARK_GRAY,
    COLOR_LIGHT_BLUE,
    COLOR_LIGHT_GREEN,
    COLOR_LIGHT_CYAN,
    COLOR_LIGHT_RED,
    COLOR_LIGHT_MAGENTA,
    COLOR_YELLOW,
    COLOR_WHITE,
};

long frame = 0;

void graphicsShutdown() {

    textmode(C80);
    clrscr();
    printf(
            "Thanks for playing!\nDOS is back with a vengeance.\n\nSource code and "
            "licenses:\nhttps://bitbucket.org/MontyOnTheRun/the-mistral-report\n\n");
}

uint8_t getPaletteEntry(uint32_t origin) {
    uint8_t shade;

    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }

    shade = 0;
    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}

void graphicsInit() {
    textmode(C80);
    clrscr();

    __dpmi_regs reg;

    reg.x.ax = 0x13;
    __dpmi_int(0x10, &reg);

    outp(0x03c8, 0);

    for (int r = 0; r < 4; ++r) {
        for (int g = 0; g < 8; ++g) {
            for (int b = 0; b < 8; ++b) {
                outp(0x03c9, (r * (16)));
                outp(0x03c9, (g * (8)));
                outp(0x03c9, (b * (8)));
            }
        }
    }

    defaultFont = loadBitmap("font.img");
}

void handleSystemEvents() {

    int lastKey = 0;

    if (kbhit()) {
        char getched = getch();
        switch (getched) {
            case 'c':
                mBufferedCommand = kCommandFire3;
                break;

            case 27:
            case 'q':
                mBufferedCommand = kCommandBack;
                break;

            case 's':
                mBufferedCommand = kCommandStrafeLeft;
                break;
            case 'd':
                mBufferedCommand = kCommandStrafeRight;
                break;

            case 13:
            case 'z':
                mBufferedCommand = kCommandFire1;
                break;

            case 'x':
            case ' ':
                mBufferedCommand = kCommandFire2;
                break;

            case 224:
            case 0: {
                char arrow = getch();
                switch (arrow) {
                    case 75:
                        mBufferedCommand = kCommandLeft;
                        break;
                    case 72:
                        mBufferedCommand = kCommandUp;
                        break;
                    case 77:
                        mBufferedCommand = kCommandRight;
                        break;
                    case 80:
                        mBufferedCommand = kCommandDown;
                        break;
                }
            }
                break;
        }
    }
}

void graphicsPut(int x, int y, uint8_t pixel ) {
    x = x * 2;
    framebuffer[(320 * y) + x] = pixel;
    framebuffer[(320 * y) + x + 1] = pixel;
}

void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t pixel ) {

    if (x0 == x1) {

        int16_t _y0 = y0;
        int16_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        for (int16_t y = _y0; y <= _y1; ++y) {
            if (x0 < 0 || x0 >= 256 || y < 0 || y >= 128) {
                continue;
            }

            graphicsPut(x0, y, pixel );
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
                continue;
            }

            graphicsPut(x, y0, pixel );
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
            framebuffer[(320 * y0) + (2 * x0)] = pixel;
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
    fix_line(x0, y, x1, y, pixel);
}

void graphicsVerticalLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel ) {
    fix_line(x0, y0, x0, y1, pixel);
}


void flipRenderer() {
    dosmemput(&framebuffer[dirtyLineY0 * 320], 320 * 200, 0xa0000 + (dirtyLineY0 * 320));
}

void clear() {}
