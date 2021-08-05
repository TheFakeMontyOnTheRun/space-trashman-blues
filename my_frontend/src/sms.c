#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cv.h>
#include <cvu.h>
#include <assert.h>

#define SMS
#define DONT_INCLUDE

#ifndef _CV_COMMON_H
#define _CV_COMMON_H

/* VRAM map
 0x0000 - 0x17ff character pattern table
 0x1800 - 0x1aff image table
 0x2000 - 0x37ff color table
 0x3800 - 0x3bff sprite pattern table
 0x3c00 - 0x3fff sprite attribute table
 */

#define PATTERN        ((const cv_vmemp)0x0000)
#define IMAGE        ((const cv_vmemp)0x1800)
#define COLOR        ((const cv_vmemp)0x2000)
#define SPRITE_PATTERNS ((const cv_vmemp)0x3800)
#define SPRITES        ((const cv_vmemp)0x3c00)

#ifndef COLS
#define COLS 32
#endif

#ifndef ROWS
#define ROWS 24
#endif

typedef unsigned char byte;
typedef signed char sbyte;
typedef unsigned short word;

#ifdef CV_CV
uintptr_t __at(0x6a) font_bitmap_a;
uintptr_t __at(0x6c) font_bitmap_0;
#endif

#ifdef CV_SMS
extern char font_bitmap_a[];
extern char font_bitmap_0[];
#endif

#define COLOR_FGBG(fg,bg) (((fg)<<4)|(bg))
#define COLOR_FG(fg) (((fg)<<4))

#ifndef LOCHAR
#define LOCHAR 0x0
#endif

#define CHAR(ch) (ch-LOCHAR)

#define wait_vsync() __asm__("halt")

struct ObjectNode* focusedItem;

volatile uint_fast8_t vint_counter;

void vint_handler(void) {
    vint_counter++;
}

const unsigned char reverse_lookup[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe, 0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };

byte reverse_bits(byte n) {
    return (reverse_lookup[n&0b1111] << 4) | reverse_lookup[n>>4];
}

void flip_sprite_patterns(word dest, const byte* patterns, word len) {
    word i;
    for (i=0; i<len; i++) {
        cvu_voutb(reverse_bits(*patterns++), dest++ ^ 16); // swap left/right chars
    }
}

void clrscr() {
    cvu_vmemset(IMAGE, 0, COLS*ROWS);
}

word getimageaddr(byte x, byte y) {
    return IMAGE + y*COLS + x;
}

byte getcharxy(byte x, byte y) {
    return cvu_vinb(getimageaddr(x,y));
}

void putcharxy(byte x, byte y, byte attr) {
    cvu_voutb(attr, getimageaddr(x,y));
}

void putstringxy(byte x, byte y, const char* string) {
    while (*string) {
        putcharxy(x++, y, CHAR(*string++));
    }
}

void delay(byte i) {
    while (i--) {
        wait_vsync();
    }
}

byte rndint(byte a, byte b) {
    return ((byte)rand() % (b-a+1)) + a;
}

void memset_safe(void* _dest, char ch, word size) {
    byte* dest = _dest;
    while (size--) {
        *dest++ = ch;
    }
}

char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h) {
    return ((byte)(x-x0) < w && (byte)(y-y0) < h); // unsigned
}

void draw_bcd_word(byte x, byte y, word bcd) {
    byte j;
    x += 3;
    for (j=0; j<4; j++) {
        putcharxy(x, y, CHAR('0'+(bcd&0xf)));
        x--;
        bcd >>= 4;
    }
}

// add two 16-bit BCD values
word bcd_add(word a, word b) __naked {
a; b; // to avoid warning
__asm
push	ix
ld	ix,#0
add	ix,sp
ld	a,4 (ix)
add	a, 6 (ix)
daa
        ld	c,a
        ld	a,5 (ix)
adc	a, 7 (ix)
daa
        ld	b,a
        ld	l, c
        ld	h, b
        pop	ix
        ret
__endasm;
}



//#link "fonts.s"


void vdp_setup() {
    cv_set_screen_active(false);
    cv_set_screen_mode(CV_SCREENMODE_STANDARD);
    cv_set_image_table(IMAGE);
    cv_set_character_pattern_t(PATTERN);
    cv_set_color_table(COLOR);
    cv_set_sprite_pattern_table(SPRITE_PATTERNS);
    cv_set_sprite_attribute_table(SPRITES);
    cv_set_sprite_big(true);
}

void set_shifted_pattern(const byte* src, word dest, byte shift) {
    byte y;
    for (y=0; y<8; y++) {
        byte a = src[y+8];
        byte b = src[y];
        cvu_voutb(a>>shift, dest);
        cvu_voutb(b>>shift | a<<(8-shift), dest+8);
        cvu_voutb(b<<(8-shift), dest+16);
        dest++;
    }
}

void copy_default_character_set() {
#ifdef CV_MSX
    static byte __at(0xf91f) CGPNT;
  static byte* __at(0xf920) CGADDR;
  cvu_memtovmemcpy(PATTERN, CGADDR, 256*8);
#else
    cvu_memtovmemcpy(PATTERN, (void *)(font_bitmap_0 - '0'*8), 256*8);
#endif
}

extern byte reverse_bits(byte n);
extern void flip_sprite_patterns(word dest, const byte* patterns, word len);

extern char cursor_x;
extern char cursor_y;

extern void clrscr();

extern word getimageaddr(byte x, byte y);
extern byte getcharxy(byte x, byte y);
extern void putcharxy(byte x, byte y, byte attr);
extern void putstringxy(byte x, byte y, const char* string);
extern void delay(byte i);
extern byte rndint(byte a, byte b);

extern void memset_safe(void* _dest, char ch, word size);
extern char in_rect(byte x, byte y, byte x0, byte y0, byte w, byte h);
// print 4-digit BCD value
extern void draw_bcd_word(byte x, byte y, word bcd);

extern void vdp_setup();
extern void set_shifted_pattern(const byte* src, word dest, byte shift);

extern void copy_default_character_set();

#endif

void setup_mode2() {
    cvu_vmemset(0, 0, 0x4000);
    cv_set_screen_mode(CV_SCREENMODE_BITMAP); // mode 2
    cv_set_image_table(IMAGE);
    cv_set_character_pattern_t(PATTERN|0x1fff); // AND mask
    cv_set_color_table(COLOR|0x1fff); // AND mask
    cv_set_sprite_attribute_table(0x2800);
    {
        byte i=0;
        do {
            cvu_voutb(i, IMAGE+i);
            cvu_voutb(i, IMAGE+0x100+i);
            cvu_voutb(i, IMAGE+0x200+i);
        } while (++i);
    }
}

uint8_t buffer[64 * 64];

void init() {
    setup_mode2();
    cv_set_screen_active(true);
}

char *menuItems[] = {
        "Use/Toggle current item",
        "Use current item with...",
        "Pick",
        "Drop",
        "Next item in inventory",
        "Next room item in focus",
        "Toogle item desc/room desc",
        "Back to room",
};

void setup_text_mode() {
    // set screen mode to text
    cv_set_screen_mode(CV_SCREENMODE_TEXT);
    // set image table address, which defines grid of characters
    cv_set_image_table(IMAGE);
    // set pattern table address, which defines character graphics
    cv_set_character_pattern_t(PATTERN);
    // clear VRAM to all zeroes
    cvu_vmemset(0, 0, 0x4000);
    // copy default character set from ROM to VRAM
    copy_default_character_set();
}

void graphicsFlush();

void renderScene();

int cooldown = 0;
int cursorPosition = 0;


void show_text(int x, int y, char *text) {

    unsigned int len = strlen(text);
    // set background and foreground colors
    // fill image table with '.' characters
    // draw message at row 0, column 1
    cvu_memtovmemcpy(IMAGE + (40 * y) + x, text, len);
    // turn on display
    cv_set_screen_active(true);
}

void pauseMenu() {
    int keepGoing = 1;
    int refresh = 1;
    int itemDesc = 1;
    struct Room* room = getRoom(getPlayerRoom());
    struct ObjectNode* roomItem = room->itemsPresent;

    setup_text_mode();

    cv_set_colors(CV_COLOR_LIGHT_GREEN, CV_COLOR_BLACK);

    while (keepGoing) {
        struct cv_controller_state state;

        if (refresh) {
            int i = 0;
            struct Item* item = getItem(focusedItem->item);
            refresh = 0;

            cvu_vmemset(IMAGE, ' ', 40 * 24);


            show_text(1, 1, "Object at room:");

            if (roomItem) {
                show_text(16, 1, getItem(roomItem->item)->description);
            }

            if (itemDesc) {
                if (item->active) {
                    show_text(1, 2, "*");
                }

                show_text(2, 2, item->description);
                show_text(1, 3, item->info);
            } else {
                show_text(1, 2, " ");
                show_text(2, 2, room->description);
                show_text(1, 3, room->info);
            }


            for (i = 0; i < 8; ++i) {
                if (i == cursorPosition) {
                    show_text(1, 16 + i, ">");
                }
                show_text(2, 16 + i, menuItems[i]);
            }
            cooldown = 4000;
        }

        cooldown--;

        if (cooldown < 0) {
            cooldown = 0;
            cv_get_controller_state(&state, 0);

/*
char *menuItems[] = {
 0       "Use/Toggle current item",
 1       "Use current item with...",
 2       "Pick",
 3       "Drop",
 4       "Next item in inventory",
 5       "Next room item in focus",
 6       "Toogle item desc/room desc",
 7       "Back to room",
};
*/
            if (state.joystick & CV_FIRE_0) {
                switch (cursorPosition) {
                    case 0: {
                        useObjectNamed(getItem(focusedItem->item)->description);
                        break;
                    }

                    case 1: {
                        struct Item *item = NULL;
                        struct Item* itemToPick = NULL;

                        if (roomItem != NULL) {
                            itemToPick = getItem(roomItem->item);
                            if (itemToPick != NULL ) {
                                if (focusedItem != NULL) {
                                    item = getItem(focusedItem->item);
                                    if (item != NULL) {

                                        /* next items */
                                        focusedItem = focusedItem->next;
                                        if (!focusedItem) {
                                            focusedItem = getPlayerItems();
                                        }

                                        item->useWithCallback(item, itemToPick);
                                    }
                                }
                            }
                        }
                    }
                        break;

                    case 2: {
                        if (roomItem != NULL) {
                            struct Item* itemToPick = getItem(roomItem->item);
                            if (itemToPick != NULL ) {
                                pickObject(itemToPick);
                                roomItem = room->itemsPresent;
                            }
                        }
                    }
                        break;

                    case 3: {
                        struct Item *item = NULL;

                        if (focusedItem != NULL) {
                            item = getItem(focusedItem->item);
                        }

                        if (item != NULL) {
                            focusedItem = focusedItem->next;
                            if (!focusedItem) {
                                focusedItem = getPlayerItems();
                            }

                            dropObjectToRoom(getPlayerRoom(), item);
                            roomItem = room->itemsPresent;
                        }
                    }
                        break;
                    case 4: {
                        struct Item *item = NULL;
                        struct ObjectNode *head = NULL;
                        head = getPlayerItems();
                        item = NULL;

                        if (head != NULL) {
                            item = getItem(head->item);
                        }

                        if (item != NULL) {
                            focusedItem = focusedItem->next;
                            if (!focusedItem) {
                                focusedItem = getPlayerItems();
                            }
                        }
                    }
                        break;

                    case 5: {
                        roomItem = roomItem->next;
                        if (!roomItem) {
                            roomItem = room->itemsPresent;
                        }
                    }
                        break;

                    case 6:
                        itemDesc = !itemDesc;
                        break;
                    case 7:
                        keepGoing = 0;
                        break;


                }
                refresh = 1;
            }

            if (state.joystick & CV_FIRE_1) {
                focusedItem = focusedItem->next;
                if (!focusedItem) {
                    focusedItem = getPlayerItems();
                }
                refresh = 1;
            }


            if (state.joystick & CV_UP) {
                cursorPosition--;
                refresh = 1;
                if (cursorPosition < 0) {
                    cursorPosition = 0;
                }
            }

            if (state.joystick & CV_DOWN) {
                cursorPosition++;
                refresh = 1;
                if (cursorPosition >= 8) {
                    cursorPosition = 7;
                }
            }
        }
    }
}


uint8_t getKey () {
    struct cv_controller_state state;
    cv_get_controller_state(&state, 0);
    
    if (state.joystick & CV_UP) {
        return 'w';
    }

    if (state.joystick & CV_LEFT) {
        if (state.joystick & CV_FIRE_1) {
            return 'a';
        }
        return 'q';
    }


    if (state.joystick & CV_RIGHT) {
        if (state.joystick & CV_FIRE_1) {
            return 'd';
        }
        return 'e';
    }

    if (state.joystick & CV_DOWN) {
        return 's';
    }

    if (state.joystick & CV_FIRE_0) {
        pauseMenu();
        clrscr();
        init();
        renderScene();
        graphicsFlush();
    }
    
    return '.';
}

void shutdownGraphics() {
}

void clearGraphics() {
    /*
     cvu_vmemset(0, 0, 0x4000 / 3 );
     
     
     {
     byte i=0;
     do {
     cvu_voutb(i, IMAGE+i);
     } while (++i);
     }*/
}

void graphicsFlush() {
    uint8_t x, y, _x, _y;
    byte b;
    word ofs, _yoff;
    uint8_t *ptr = &buffer[0];
    uint8_t pixel;
    _y = 2;
    for (y = 64; y; --y ) {
        _y++;
        _yoff = ( ((_y >> 3 ) << 8)) + (_y & 7);
        
        _x = 96;
        
        for (x = 0; x < 64;) {
            
            // refactor: _x & 248 -> (_x / 8 ) * 8
            ofs = ( _x & 248 ) + _yoff;
            b = cvu_vinb(PATTERN + ofs);
            
            //1
            pixel = *ptr;
            if ( pixel & 1 ) {
                
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
                
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //2
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //3
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
                
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            //4
            pixel = *ptr;
            if ( pixel & 1 ) {
                if ( ~(pixel & 2) ) {
                    b |= 128 >> (_x & 7);
                    
                }
            } else {
                if ( (pixel & 2) ) {
                    b &= ~(128 >> ( _x     &7));
                    
                }
            }
            *ptr = pixel << 1;
            ptr++;
            _x += 1;
            
            x += 4;
            cvu_voutb(b, PATTERN + ofs);
            cvu_voutb(32, COLOR + ofs);
            
            
        }
    }
}

void hLine(uint8_t x0, uint8_t x1, uint8_t y0) {
    uint8_t *ptr;
    uint8_t _x0 = x0;
    uint8_t _x1 = x1;
    
    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }
    
    ptr = &buffer[(y0 * 64) + _x0];
    for (uint8_t x = _x0; x <= _x1; ++x) {
        {
            ptr++;
            *ptr |= 1;
            
        }
    }
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1) {
    
    uint8_t *ptr;
    uint8_t _y0 = y0;
    uint8_t _y1 = y1;
    
    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }
    
    ptr = &buffer[(_y0 * 64) + x0];
    
    for (uint8_t y = _y0; y <= _y1; ++y) {
        *ptr |= 1;
        ptr += 64;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    buffer[(y * 64) + x ] |= 1;
}

int putchar(int dummy) {
    dummy = dummy;
}

void drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    word offset;
    uint8_t *ptr;
    if (x0 == x1) {
        
        uint8_t _y0 = y0;
        uint8_t _y1 = y1;
        
        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }
        
        
        offset = (_y0 * 64) + x0;
        ptr = &buffer[offset];
        
        for (uint8_t y = _y0; y <= _y1; ++y) {
            
            
            {
                ptr += 64;
                *ptr |= 1;
                
            }
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
        
        
        offset = (y0 * 64) + _x0;
        ptr = &buffer[offset];
        for (uint8_t x = _x0; x <= _x1; ++x) {
            {
                ptr++;
                *ptr |= 1;
                
            }
        }
        return;
    }
    
    
    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
        
        uint8_t dx = abs(x1 - x0);
        int8_t sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0);
        int8_t sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy;  /* error value e_xy */
        int16_t e2;
        word offset = (y0 * 64) + x0;
        
        if (sy > 0 ) {
            while (1) {
                
                buffer[offset] |= 1;
                
                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0++;
                    offset += 64;
                }
            }
        } else {
            while (1) {
                
                buffer[offset] |= 1;
                
                /* loop */
                if (x0 == x1 && y0 == y1) return;
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                    offset += sx;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0--;
                    offset -= 64;
                }
            }
        }
        
    }
}

void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    x0 = x1 = y0 = y1;
}

