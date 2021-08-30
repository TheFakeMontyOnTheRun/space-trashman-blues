#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cpctelera.h>

uint8_t __at(0xC000)  screen[16 * 1024];

uint8_t* baseScreen = 0xC000;

#ifdef RES64X128
const uint16_t lineStart[128] = {
        0,
        2048,
        4096,
        6144,
        8192,
        10240,
        12288,
        14336,
        80,
        2128,
        4176,
        6224,
        8272,
        10320,
        12368,
        14416,
        160,
        2208,
        4256,
        6304,
        8352,
        10400,
        12448,
        14496,
        240,
        2288,
        4336,
        6384,
        8432,
        10480,
        12528,
        14576,
        320,
        2368,
        4416,
        6464,
        8512,
        10560,
        12608,
        14656,
        400,
        2448,
        4496,
        6544,
        8592,
        10640,
        12688,
        14736,
        480,
        2528,
        4576,
        6624,
        8672,
        10720,
        12768,
        14816,
        560,
        2608,
        4656,
        6704,
        8752,
        10800,
        12848,
        14896,
        640,
        2688,
        4736,
        6784,
        8832,
        10880,
        12928,
        14976,
        720,
        2768,
        4816,
        6864,
        8912,
        10960,
        13008,
        15056,
        800,
        2848,
        4896,
        6944,
        8992,
        11040,
        13088,
        15136,
        880,
        2928,
        4976,
        7024,
        9072,
        11120,
        13168,
        15216,
        960,
        3008,
        5056,
        7104,
        9152,
        11200,
        13248,
        15296,
        1040,
        3088,
        5136,
        7184,
        9232,
        11280,
        13328,
        15376,
        1120,
        3168,
        5216,
        7264,
        9312,
        11360,
        13408,
        15456,
        1200,
        3248,
        5296,
        7344,
        9392,
        11440,
        13488,
        15536,
};
#else
const uint16_t lineStart[64] = {
        0,
        2048,
        4096,
        6144,
        8192,
        10240,
        12288,
        14336,
        80,
        2128,
        4176,
        6224,
        8272,
        10320,
        12368,
        14416,
        160,
        2208,
        4256,
        6304,
        8352,
        10400,
        12448,
        14496,
        240,
        2288,
        4336,
        6384,
        8432,
        10480,
        12528,
        14576,
        320,
        2368,
        4416,
        6464,
        8512,
        10560,
        12608,
        14656,
        400,
        2448,
        4496,
        6544,
        8592,
        10640,
        12688,
        14736,
        480,
        2528,
        4576,
        6624,
        8672,
        10720,
        12768,
        14816,
        560,
        2608,
        4656,
        6704,
        8752,
        10800,
        12848,
        14896
};
#endif

void shutdownGraphics() {
}

void clearGraphics();

void graphicsPut(uint8_t nColumn, uint8_t nLine);

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg);

uint8_t getKey() {

    cpct_scanKeyboard_f();
    if (cpct_isKeyPressed(Key_CursorLeft) || cpct_isKeyPressed(Key_Q))
        return 'q';

    if (cpct_isKeyPressed(Key_CursorRight) || cpct_isKeyPressed(Key_E))
        return 'e';

    if (cpct_isKeyPressed(Key_CursorUp) || cpct_isKeyPressed(Key_W))
        return 'w';

    if (cpct_isKeyPressed(Key_CursorDown) || cpct_isKeyPressed(Key_S))
        return 's';

    if (cpct_isKeyPressed(Key_P) || cpct_isKeyPressed(Key_D))
        return 'd';

    if (cpct_isKeyPressed(Key_O) || cpct_isKeyPressed(Key_A))
        return 'a';



    if (cpct_isKeyPressed(Key_O) || cpct_isKeyPressed(Key_A))
        return 'a';


    if (cpct_isKeyPressed(Key_L))
        return 'l';

    if (cpct_isKeyPressed(Key_K))
        return 'k';

    return '.';
}

void init() {
    cpct_disableFirmware();
    cpct_setVideoMode(0);
    cpct_setVideoMemoryPage(cpct_pageC0);
    baseScreen = (uint8_t*)0xC000;
    cpct_memset_f64((uint8_t*)baseScreen, 0, 16 * 1024);
}

void graphicsFlush() {
}

void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    if (x0 == x1) {

        uint8_t _y0 = y0;
        uint8_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        for (uint8_t y = _y0; y <= _y1; ++y) {
            if (y >= 128) {
                continue;
            }

            graphicsPut(x0, y);
        }
        return;
    }

    if (y0 == y1) {
        uint8_t _x0 = x0;
        uint8_t _x1 = x1;

        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }

        for (uint8_t x = _x0; x <= _x1; ++x) {
            if (y0 >= 128) {
                continue;
            }

            graphicsPut(x, y0);
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

        int8_t dx = abs(x1 - x0);
        int8_t sx = x0 < x1 ? 1 : -1;
        int8_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int8_t err = dx + dy;  /* error value e_xy */
        int8_t e2;
        while (1) {
            graphicsPut(x0, y0);
            /* loop */
            if (x0 == x1 && y0 == y1) return;
            e2 = 2 * err;

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

void hLine(uint8_t x0, uint8_t x1, uint8_t y) {

    unsigned char *pS;
    unsigned char *base;
    uint8_t nLine = y;
    uint8_t nColumn = 0;
    uint8_t bytes;
    uint8_t dx = (x1 - x0);
    bytes = dx >> 1;
    base = (unsigned char *) baseScreen + lineStart[nLine] + (x0 >> 1);
//write whole bytes first, then the remainder with masks

    if (x0 & 1) {
        graphicsPut(x0, nLine);
        base++;
        dx--;
        bytes--;
        x0++;
    }

    memset(base, 0xFF, bytes);

    base += bytes;
    dx -= (bytes << 1);

    for (nColumn = 0; nColumn <= dx; nColumn++) {

        unsigned char nByte;
        pS = base + (nColumn >> 1);
        nByte = *pS;

        if (nColumn & 1) {
            nByte &= 170;
            nByte |= 64;
        } else {
            nByte &= 85;
            nByte |= 128;
        }

        *pS = nByte;
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {

    register uint8_t y;
    register uint16_t *lineStartPtr;
    register uint8_t *pS;
    register uint8_t *base;
    register uint8_t nByte;

    base = baseScreen + (x0 >> 1);

    if (y0 > y1) {
        lineStartPtr = &lineStart[y1];
        y = y0 - y1;
    } else {
        lineStartPtr = &lineStart[y0];
        y = y1 - y0;
    }

    if (x0 & 1) {
        while( y-- ) {
            pS = base + *lineStartPtr++;
            nByte = *pS;

            nByte &= 170;
            nByte |= 64;

            *pS = nByte;
        }
    } else {
        while( y-- ) {
            pS = base + *lineStartPtr++;
            nByte = *pS;

            nByte &= 85;
            nByte |= 128;

            *pS = nByte;
        }
    }
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg) {
    unsigned char nPixel = fg + bg; /* just to silence the compiler warnings */
    uint8_t *pS;

    if (nColumn >= 128 || nLine >= 128) {
        return;
    }

    nPixel = nColumn & 1;

    pS = (unsigned char *) baseScreen +lineStart[nLine] + (nColumn >> 1);

    cpct_drawStringM0(str, pS);
}

inline void graphicsPut(uint8_t nColumn, uint8_t nLine) {

    unsigned char *pS;
    unsigned char nByte = 0;

    pS = (unsigned char *) baseScreen + lineStart[nLine] + (nColumn >> 1);
    nByte = *pS;

    if (nColumn & 1) {
        nByte &= 170;
        nByte |= 64;
    } else {
        nByte &= 85;
        nByte |= 128;
    }

    *pS = nByte;
}

void clearGraphics() {
    memset((uint8_t*)baseScreen, 0, 16 * 1024);
}
