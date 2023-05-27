
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

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#define NES_MAPPER 2		// UxROM mapper
#define NES_CHR_BANKS 0		// CHR RAM

int ppu_is_on = 0;

// simple 6502 delay loop (5 cycles per loop)
#define DELAYLOOP(n) \
  __asm__("ldy #%b", n); \
  __asm__("@1: dey"); \
  __asm__("bne @1");

extern const struct Pattern patterns[127];

extern int8_t map[32][32];

extern struct ObjectNode *focusedItem;

extern struct ObjectNode *roomItem;

extern uint8_t accessGrantedToSafe;

void clearGraphics(void);

#define BUFFER_SIZEX 16
#define BUFFER_SIZEY 128
#define BUFFER_RESX 128
#define BUFFER_RESY 128
#define COOLDOWN_MAX 0x2EF
#define MARGIN_TEXT_SCREEN_LIMIT 30
/*
uint8_t font[] = {
        // ASCII table starting on SPACE.
        // Being on line 32 is no accident.
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // space
        , 0x10, 0x38, 0x38, 0x10, 0x10, 0x00, 0x10, 0x00, 0x6c, 0x6c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
        0x7c, 0x28, 0x28, 0x7c, 0x28, 0x00, 0x20, 0x38, 0x40, 0x30, 0x08, 0x70, 0x10, 0x00, 0x64, 0x64, 0x08, 0x10,
        0x20, 0x4c, 0x4c, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00,
        0x00, 0x28, 0x38, 0x7c, 0x38, 0x28, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x30, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00 // /space - 15
        , 0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x38, 0x00 // 0
        , 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x38, 0x44, 0x04, 0x18, 0x20, 0x40, 0x7c, 0x00, 0x38, 0x44,
        0x04, 0x38, 0x04, 0x44, 0x38, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x04, 0x44, 0x38, 0x00, 0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x04, 0x08, 0x10, 0x20, 0x20,
        0x20, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x38, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30, 0x00,
        0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x08, 0x10,
        0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04,
        0x08, 0x10, 0x20, 0x00, 0x38, 0x44, 0x04, 0x18, 0x10, 0x00, 0x10, 0x00, 0x38, 0x44, 0x5c, 0x54, 0x5c, 0x40,
        0x38, 0x00 // /0
        , 0x38, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00 // a
        , 0x78, 0x44, 0x44, 0x78, 0x44, 0x44, 0x78, 0x00, 0x38, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 0x78, 0x44,
        0x44, 0x44, 0x44, 0x44, 0x78, 0x00, 0x7c, 0x40, 0x40, 0x78, 0x40, 0x40, 0x7c, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x40, 0x5c, 0x44, 0x44, 0x3c, 0x00, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44,
        0x44, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x04, 0x04, 0x04, 0x04, 0x44, 0x44, 0x38, 0x00,
        0x44, 0x48, 0x50, 0x60, 0x50, 0x48, 0x44, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7c, 0x00, 0x44, 0x6c,
        0x54, 0x44, 0x44, 0x44, 0x44, 0x00, 0x44, 0x64, 0x54, 0x4c, 0x44, 0x44, 0x44, 0x00, 0x38, 0x44, 0x44, 0x44,
        0x44, 0x44, 0x38, 0x00, 0x78, 0x44, 0x44, 0x78, 0x40, 0x40, 0x40, 0x00, 0x38, 0x44, 0x44, 0x44, 0x54, 0x48,
        0x34, 0x00, 0x78, 0x44, 0x44, 0x78, 0x48, 0x44, 0x44, 0x00, 0x38, 0x44, 0x40, 0x38, 0x04, 0x44, 0x38, 0x00,
        0x7c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 0x44, 0x44,
        0x44, 0x44, 0x44, 0x28, 0x10, 0x00, 0x44, 0x44, 0x54, 0x54, 0x54, 0x54, 0x28, 0x00, 0x44, 0x44, 0x28, 0x10,
        0x28, 0x44, 0x44, 0x00, 0x44, 0x44, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00, 0x78, 0x08, 0x10, 0x20, 0x40, 0x40,
        0x78, 0x00, 0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00, 0x00, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00,
        0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00, 0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x30, 0x30, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x08,
        0x30, 0x40, 0x78, 0x00, 0x18, 0x20, 0x20, 0x60, 0x20, 0x20, 0x18, 0x00, 0x10, 0x10, 0x10, 0x00, 0x10, 0x10,
        0x10, 0x00, 0x30, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x30, 0x00, 0x28, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x10, 0x38, 0x6c, 0x44, 0x44, 0x7c, 0x00, 0x00
};
*/
uint8_t cursorPosition = 0;

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

void init(void) {
    monobitmap_setup();
    pal_bg(MONOBMP_PALETTE);
    cooldown = COOLDOWN_MAX;
}

char *menuItems[] = {
        "Use/Toggle",
        "Use with...",
        "Use/pick...",
        "Drop",
        "Next item",
        "Next in room",
};

void writeStrWithLimit(uint8_t _x, uint8_t y, const char *text, uint8_t limitX) {
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

void writeStr(uint8_t _x, uint8_t y, const char *text, uint8_t fg, uint8_t bg) {
    writeStrWithLimit(_x, y, text, MARGIN_TEXT_SCREEN_LIMIT);
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {}

void showMessage(const char *message) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, message, 2, 0);
    writeStr(2, 22, "Press B button to continue", 2, 0);

    while (keepGoing) {
        if (getKey() == 'p') {
            keepGoing = 0;
        }
    }

    //backToGraphics();
}

void titleScreen(void) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, "Sub Mare Imperium: Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 6, " Press B to start ", 2, 0);

    while (keepGoing) {
        if (getKey() == 'p') {
            keepGoing = 0;
        }
    }
  //  backToGraphics();
}

void refreshJustGraphics(void) {
    clearGraphics();
    renderScene();
    graphicsFlush();
}

void backToGraphics(void) {
    clearScreen();
    HUD_initialPaint();
    refreshJustGraphics();
}

void performAction(void) {
    switch (getGameStatus()) {
        case kBadVictory:
            showMessage("Victory! Too bad you didn't survive");
            while (1);

        case kBadGameOver:
            showMessage("You're dead! And so are the\n"
                        "other people on the path of\n"
                        "destruction faulty reactor");
            while (1);

        case kGoodVictory:
            showMessage("Victory! You managed to destroy the\nship and get out alive");
            while (1);

        case kGoodGameOver:
            showMessage("You failed! While you're alive\n"
                        "you failed to prevent the worst\n"
                        "scenario and now EVERYBODY is\n"
                        "dead!)");
            while (1);

        default:
        case kNormalGameplay:
            break;
    }

/*
char *menuItems[] = {
 0       "Use/Toggle current item",
 1       "Use current item with...",
 2       "Pick",
 3       "Drop",
 4       "Next item in inventory",
 5       "Next room item in focus",
};
*/

    switch (cursorPosition) {
        case 0:
            useObjectNamed(getItem(focusedItem->item)->name);
            break;
        case 1:
            interactWithItemInRoom();
            HUD_refresh();
            break;
        case 2:
            pickItem();
            refreshJustGraphics();
            HUD_refresh();
            break;
        case 3:
            dropItem();
            refreshJustGraphics();
            HUD_refresh();
            break;
        case 4:
            nextItemInHand();
            break;
        case 5:
            nextItemInRoom();
            break;
    }
}

void clearTextScreen(void) {
    clearScreen();
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

uint8_t getKey(void) {

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

void graphicsFlush(void) {
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


    writeStrWithLimit(17, 12, "Direction: ", 31);

    switch (getPlayerDirection()) {
        case 0:
            writeStrWithLimit(29, 12, "N", 31);
            break;
        case 1:
            writeStrWithLimit(29, 12, "E", 31);
            break;
        case 2:
            writeStrWithLimit(29, 12, "S", 31);
            break;
        case 3:
            writeStrWithLimit(29, 12, "W", 31);
            break;
    }
    ppu_on_all();
    memset(&buffer[0], 0, BUFFER_SIZEX * BUFFER_SIZEY);
}

void sleepForMS(uint32_t ms) {
    //we cant afford to sleep
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


uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t *ptr) {

#ifdef HALF_BUFFER
    x = x >> 1;
	y = y >> 1;
#endif

    if (ptr == NULL) {
        /*
        // originally was this
        ptr = &buffer[(16 * 8 * (y >> 3)) + //skip the entire row of patterns along the y
                      (8 * (x >> 3)) + //skip to the correct pattern in the row
                      (y & 7)]; //skip to the line in pattern
        */
        ptr = &buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)];
    }

    *ptr |= (128 >> (x & 7));

    return ptr;
}

void graphicsPut(uint8_t x, uint8_t y) {

#ifdef HALF_BUFFER
    x = x >> 1;
	y = y >> 1;
#endif

    /*
     // was originally this
    uint8_t *ptr = &buffer[(16 * 8 * (y >> 3) + //skip the entire row of patterns along the y
                           (8 * (x >> 3)) + //skip to the correct pattern in the row
                           (y & 7)]; //skip to the line in pattern
    */

    buffer[((y & ~7) << 4) + (x & ~7) + (y & 7)] |= (128 >> (x & 7));
}


void HUD_initialPaint(void) {
    uint8_t i;

    for (i = 0; i < 6; ++i) {
        writeStr(18, 14 + i, menuItems[i], 2, 0);
    }

    HUD_refresh();
}

void HUD_refresh(void) {
    uint8_t d;
    uint8_t i;

    for (d = 0; d < 15; ++d) {
        writeStr(17 + d, 2, " ", 2, 0);
        writeStr(17 + d, 3, " ", 2, 0);
        writeStr(17 + d, 6, " ", 2, 0);
        writeStr(17 + d, 7, " ", 2, 0);
    }


    for (i = 0; i < 6; ++i) {
        writeStr(17, 14 + i, (i == cursorPosition) ? ">" : " ", 2, 0);
    }

    writeStrWithLimit(17, 5, "Object in hand", 31);

    if (focusedItem != NULL) {
        struct Item *item = getItem(focusedItem->item);


        if (item->active) {
            writeStr(17, 6, "*", 2, 0);
        }

        writeStrWithLimit(18, 6, item->name, 31);
    } else {
        writeStrWithLimit(18, 6, "Nothing", 31);
    }

    writeStrWithLimit(17, 1, "Object in room", 31);

    if (roomItem != NULL) {
        struct Item *item = getItem(roomItem->item);


        if (item->active) {
            writeStrWithLimit(17, 2, "*", 31);
        }

        writeStrWithLimit(18, 2, item->name, 31);
    } else {
        writeStrWithLimit(18, 2, "Nothing", 31);
    }
}
