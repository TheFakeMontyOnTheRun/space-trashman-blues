
/*
Creates a monochrome frame buffer in video RAM.
We map the pattern tables to CHR RAM, using the UxROM (2) mapper.
By cleverly setting up palettes, and using a split-screen
CHR bank switch, we split the screen into four different regions
that display their own pixels.
*/

#include "neslib/neslib.h"
#include <nes.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#include "Enums.h"
#include "Core.h"
#include "Renderer.h"

#include "GamepadUI.h"
#include "UI.h"
#include "font.h"

#define NES_MAPPER 2		// UxROM mapper
#define NES_CHR_BANKS 0		// CHR RAM

int ppu_is_on = 0;

// simple 6502 delay loop (5 cycles per loop)
#define DELAYLOOP(n) \
  __asm__("ldy #%b", n); \
  __asm__("@1: dey"); \
  __asm__("bne @1");

void clearGraphics(void);

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 30

uint8_t buffer[BUFFER_SIZEX * BUFFER_SIZEY];
uint16_t cooldown;

// call every frame to split screen
void monobitmap_split(void) {
    // split screen at line 128
    split(0,0);
    DELAYLOOP(15); // delay until end of line
    PPU.control = PPU.control ^ 0x10; // bg bank 1
}



// write values 0..255 to nametable
void monobitmap_put_256inc() {
    uint16_t i;
    for (i=0; i<256; i++)
        vram_put(i);
}

// sets up attribute table
void monobitmap_put_attrib() {
    vram_fill(0x00, 0x10); // first palette
    vram_fill(0x55, 0x10); // second palette
}

void clearScreen(void) {
    vram_adr(0x0);
    vram_fill(0x0, 0x2000);
}

void monobitmap_set_pixel(uint8_t x, uint8_t y, uint8_t color);


// sets up PPU for monochrome bitmap
void monobitmap_setup() {
    clearScreen();
    // setup nametable A and B
    vram_adr(NAMETABLE_A);
    monobitmap_put_256inc();
    monobitmap_put_256inc();
    monobitmap_put_256inc();
    monobitmap_put_256inc();
    vram_adr(NAMETABLE_A + 0x3c0);
    monobitmap_put_attrib();
    monobitmap_put_attrib();
    bank_bg(0);
    // setup sprite 0
    bank_spr(1);
    oam_clear();
    oam_size(0);
    oam_spr(247, 125, 255, 0, 0);
    // draw a pixel for it to collide with
    monobitmap_set_pixel(247, 126, 1);
    // make sprite 255 = white line
    vram_adr(0x1ff0);
    vram_fill(0xff, 0x1);
}

/*{pal:"nes",layout:"nes"}*/
const uint8_t MONOBMP_PALETTE[16] = {
        0x03,
        0x30, 0x03, 0x30,  0x00,
        0x03, 0x30, 0x30,  0x00,
        0x30, 0x03, 0x30,  0x00,
        0x03, 0x30, 0x30
};


// set a pixel at (x,y) color 1=set, 0=clear
void monobitmap_set_pixel(uint8_t x, uint8_t y, uint8_t color) {
    uint8_t b;
    // compute pattern table address
    int a = (x/8)*16 | ((y&63)/8)*(16*32) | (y&7);
    if (y & 64) a |= 8;
    if (y & 128) a |= 0x1000;
    // if PPU is active, wait for next frame
    if (ppu_is_on) {
        ppu_wait_nmi();
    }
    // read old uint8_t
    vram_adr(a);
    vram_read(&b, 1);
    if (color) {
        b |= 128 >> (x&7); // set pixel
    } else {
        b &= ~(128 >> (x&7)); // clear pixel
    }
    // write new uint8_t
    vram_adr(a);
    vram_put(b);
    // if PPU is active, reset PPU addr and split screen
    if (ppu_is_on) {
        vram_adr(0);
        monobitmap_split();
    }
}

void writeStrWithLimit(uint8_t _x, uint8_t y, const char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {
/*
    uint8_t len = strlen(text);
    const char *ptr = text;
    uint8_t c = 0;
    uint8_t chary = 0;
    uint8_t x = _x;
    char lastChar;
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
        //vwrite(fontTop, map_pixel(x << 3, y << 3), 8);
        ++x;
        ++ptr;
    }
    */
}
enum ECommand getInput(void) {

    unsigned int key = pad_poll(0);
    unsigned int trigger = pad_trigger(0);

    if (cooldown) {
        cooldown--;
    }

    if (key & PAD_UP) {
        return 'w';
    }

    if (key & PAD_LEFT) {
        if (trigger & PAD_B) {
            return 'a';
        } else {
            return 'q';
        }
    }

    if (key & PAD_RIGHT) {
        if (trigger & PAD_B) {
            return 'd';
        } else {
            return 'e';
        }
    }

    if (key & PAD_DOWN) {
        return 's';
    }

    if ((trigger & PAD_A) && !cooldown) {
        performAction();
        cooldown = COOLDOWN_MAX;
        return 'p';
    }

    if ((trigger & PAD_B) && !cooldown) {
        cursorPosition = (cursorPosition + 1);

        if (cursorPosition >= 6) {
            cursorPosition = 0;
        }

        HUD_refresh();
        cooldown = COOLDOWN_MAX;
        return 'p';
    }

    return '.';
}

void shutdownGraphics(void) {
}

void clearGraphics(void) {
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void endFrame(void) {
    if (needsToRedrawVisibleMeshes) {
        int x, y;
        uint8_t *ptr = &buffer[0];

        clearScreen();
        for (y = 0; y < 128; ++y ) {
            for ( x = 0; x < 128; ++x ) {
                if (ptr) {
                    monobitmap_set_pixel(x,y,1);
                }
                ++ptr;
            }
        }

        ppu_on_all();
        memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
    }
}


void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    uint8_t _y0 = y0;
    uint8_t _y1 = y1;
    uint8_t patternLine;
    uint8_t *ptr;
    uint8_t shiftXAnd7;
    uint8_t y;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    patternLine = (_y0 & 7); /* which line inside the pattern; */

    /*
    // originally was this
    ptr = &buffer[(16 * 8 * (y >> 3)) + //skip the entire row of patterns along the y
                  (8 * (x >> 3)) + //skip to the correct pattern in the row
                  (y & 7)]; //skip to the line in pattern
    */
    ptr = &buffer[((_y0 & ~7) << 4) + (x0 & ~7) + (_y0 & 7)];

    shiftXAnd7 = 128 >> (x0 & 7);

    for (y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            *ptr |= shiftXAnd7;
        }

        ++patternLine;
        ++ptr;

        if (patternLine >= 8) {
            patternLine = 0;
            ptr += (16 * 8) - 8;
        }
    }
}


void initHW(int argc, char **argv) {
    (void)argc;
    (void)argv;

    monobitmap_setup();
    pal_bg(MONOBMP_PALETTE);
    cooldown = COOLDOWN_MAX;

    initGamepadUI();
    needsToRedrawVisibleMeshes = 0;
}

void clearTextScreen(void) {
    uint8_t c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 0; d < 40; ++d) {
            writeStrWithLimit(d, c, " ", 320 / 8, 2, 0);
        }
    }
}

void handleSystemEvents(void) {}

void startFrame(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
}


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {

    buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)] |= (128 >> (x & 7));

    return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {

    /*
     // was originally this
    uint8_t *ptr = &buffer[(16 * 8 * (y >> 3) + //skip the entire row of patterns along the y
                           (8 * (x >> 3)) + //skip to the correct pattern in the row
                           (y & 7)]; //skip to the line in pattern
    */

    buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)] |= (128 >> (x & 7));
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
