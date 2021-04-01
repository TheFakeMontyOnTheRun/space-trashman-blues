#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cv.h>
#include <cvu.h>

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

extern volatile uint_fast8_t vint_counter;

extern void vint_handler(void);
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
// add two 16-bit BCD values
extern word bcd_add(word a, word b);

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

uint8_t getKey () {
    struct cv_controller_state state;
    cv_get_controller_state(&state, 0);
    
    if (state.joystick & CV_UP) {
        return 'w';
    }
    
    if (state.joystick & CV_LEFT) {
        return 'a';
    }
    
    
    if (state.joystick & CV_RIGHT) {
        return 'd';
    }
    
    
    if (state.joystick & CV_DOWN) {
        return 's';
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
    _y = 0;
    for (y = 64; y; --y ) {
        _y++;
        _yoff = ( ((_y >> 3 ) << 8)) + (_y & 7);
        
        _x = 0;
        
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
        
        
        
        ptr += 64;
        *ptr |= 1;
        
        
    }
}



#define WEDGE_TYPE_NEAR_LEFT 4
#define WEDGE_TYPE_NEAR_RIGHT 8

enum DIRECTION {
    DIRECTION_N,
    DIRECTION_E,
    DIRECTION_S,
    DIRECTION_W
};

#define IN_RANGE(V0, V1, V)  ((V0) <= (V) && (V) <= (V1))


void shutdownGraphics();

void clearGraphics();

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg);

uint8_t getKey();

void init();

void graphicsFlush();

void fix_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);

void hLine(uint8_t x0, uint8_t x1, uint8_t y);

void vLine(uint8_t x0, uint8_t y0, uint8_t y1);

int8_t stencilHigh[64];

int8_t cameraX = 33;
int8_t cameraZ = 22;
int8_t cameraRotation = 0;
uint8_t running = 1;

struct Projection {
    uint8_t px;
    uint8_t py;
    int16_t dx;
};


struct Pattern {
    uint8_t ceiling: 4;
    uint8_t elementsMask: 4;
    uint8_t geometryType;
    uint8_t block;
};

const struct Projection projections[36] =
{
    //                                   Z
    {0,  96, -64}, // 0
    {0,  64, -32}, // 1
    {11, 53, -21}, // 2
    {16, 48,  -16}, // 3
    {19, 45,  -13}, // 4
    {21, 43,  -11}, // 5
    {23, 41,  -9},  // 6
    {24, 40,  -8},  // 7
    {25, 39,  -7},  // 8
    {26, 38,  -6},  // 9
    {26, 38,  -6},  // 10
    {27, 37,  -5},  // 11
    {27, 37,  -5}, // 12
    {27, 37,  -5}, // 13
    {28, 36,  -4}, // 14
    {28, 36,  -4}, // 15
    {28, 36,  -4}, // 16
    {28, 36,  -4}, // 17
    {29, 35,  -3}, // 18
    {29, 35,  -3}, // 19
    {29, 35,  -3}, // 20
    {29, 35,  -3}, // 21
    {29, 35,  -3},  // 22
    {29, 35,  -3},  // 23
    {29, 35,  -3},  // 24
    {30, 34,  -2},  // 25
    {30, 34,  -2},  // 26
    {30, 34,  -2},  // 27
    {30, 34,  -2}, // 28
    {30, 34,  -2}, // 29
    {30, 34,  -2}, // 30
    {30, 34,  -2}, // 31
    {30, 34,  -2}, // 32
    {30, 34,  -2}, // 33
    {30, 34,  -2}, // 34
    {30, 34,  -2}, // 35
};

const struct Pattern patterns[16] = {
    {7, 3, 0, 0}, //0
    {7, 3, 0, 1}, // 1
    {0, 0, 0, 0}, // 2
    {0, 3, 0, 0}, //3
    {0, 3, 4, 0}, //4
    {0, 3, 8, 0}, //5
    {3, 3, 0, 0}, //6
    {7, 3, 0, 0}, //7
    {0, 1, 0, 0}, // 8
    {0, 3, 0, 0}, // 9
};

const int8_t map[32][32] = {
    {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 8, 7, 7, 7, 7, 7, 8, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 2, 2, 7, 0, 0, 0, 7, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 2, 7, 7, 7, 7, 7, 9, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 2, 7, 7, 8, 8, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 1, 2, 7, 7, 2, 2, 1, 1, 1, 1, 2, 7, 7, 7, 7, 7, 7, 7, 7, 2, 2, 2, 2, 1, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 2, 7, 7, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 8, 3, 3, 3, 7, 8, 2, 1, 0, 0, 0},
    {1, 2, 2, 2, 2, 2, 2, 7, 7, 2, 9, 2, 8, 2, 2, 2, 2, 2, 2, 1, 1, 2, 7, 7, 7, 7, 2, 2, 1, 0, 0, 0},
    {1, 2, 8, 7, 7, 5, 3, 6, 6, 3, 4, 7, 7, 7, 7, 7, 7, 5, 8, 1, 1, 2, 7, 7, 7, 7, 2, 1, 1, 0, 0, 0},
    {1, 2, 2, 7, 7, 7, 7, 7, 7, 7, 7, 7, 0, 0, 0, 0, 7, 7, 2, 1, 1, 2, 7, 7, 6, 7, 2, 1, 1, 0, 0, 0},
    {1, 2, 9, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 2, 7, 7, 7, 7, 2, 2, 1, 1, 1, 0},
    {1, 2, 2, 8, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 2, 7, 7, 0, 7, 9, 2, 2, 2, 1, 0},
    {1, 1, 2, 3, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 1, 9, 7, 7, 0, 7, 7, 7, 8, 2, 1, 0},
    {0, 1, 2, 3, 6, 7, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 7, 2, 1, 1, 7, 7, 7, 0, 0, 0, 7, 2, 2, 1, 0},
    {0, 1, 2, 3, 6, 7, 0, 0, 0, 0, 7, 3, 7, 0, 0, 0, 0, 7, 2, 1, 1, 0, 0, 7, 0, 0, 0, 7, 2, 1, 1, 0},
    {0, 1, 2, 2, 6, 7, 0, 0, 0, 0, 7, 7, 7, 0, 0, 0, 0, 7, 2, 1, 7, 7, 7, 7, 0, 0, 0, 7, 2, 2, 1, 0},
    {0, 1, 1, 2, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 8, 7, 0, 7, 7, 7, 9, 2, 1, 0},
    {0, 0, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 8, 2, 2, 2, 1, 0},
    {0, 0, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 2, 2, 1, 1, 1, 0},
    {0, 0, 1, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 2, 1, 2, 2, 2, 7, 0, 7, 2, 2, 1, 0, 0, 0},
    {0, 0, 1, 2, 7, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 9, 1, 2, 2, 9, 7, 7, 7, 9, 2, 1, 0, 0, 0},
    {0, 0, 1, 2, 7, 0, 0, 7, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 7, 7, 7, 8, 2, 2, 1, 0, 0, 0},
    {0, 0, 1, 2, 7, 0, 0, 7, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 7, 0, 7, 2, 2, 1, 1, 0, 0, 0},
    {0, 1, 1, 2, 7, 0, 0, 7, 2, 2, 2, 2, 9, 7, 0, 0, 7, 7, 3, 1, 2, 9, 7, 0, 7, 2, 1, 1, 0, 0, 0, 0},
    {0, 1, 2, 2, 7, 0, 0, 7, 2, 7, 7, 7, 7, 7, 0, 0, 0, 7, 7, 7, 7, 7, 7, 0, 7, 2, 2, 1, 0, 0, 0, 0},
    {0, 1, 2, 9, 7, 7, 7, 7, 2, 7, 7, 7, 7, 7, 0, 0, 0, 7, 7, 7, 7, 7, 7, 7, 7, 9, 2, 1, 0, 0, 0, 0},
    {0, 1, 2, 2, 2, 2, 2, 2, 9, 2, 2, 2, 8, 7, 0, 0, 0, 7, 8, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 7, 0, 0, 7, 7, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 7, 7, 7, 7, 8, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

int8_t max(int8_t x1, int8_t x2) {
    return x1 > x2 ? x1 : x2;
}

int8_t min(int8_t x1, int8_t x2) {
    return x1 < x2 ? x1 : x2;
}


uint8_t drawWedge(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask, uint8_t type) {
    
    int16_t z1;
    int16_t z0px;
    int16_t z1px;
    int16_t z0py;
    
    int16_t z1py;
    int16_t z0dx;
    int16_t z1dx;
    
    int16_t px0z0;
    int16_t py0z0;
    int16_t py0z1;
    
    int16_t py1z0;
    int16_t py1z1;
    
    int16_t px1z1;
    
    if (z0 >= 32) {
        return 0;
    }
    
    z1 = z0 + dZ;
    
    if (z0 <= 2) {
        return 0;
    }
    
    if (z1 <= 2) {
        return 0;
    }
    
    if (z1 >= 32) {
        return 0;
    }
    
    
    if (type == WEDGE_TYPE_NEAR_LEFT) {
        z0px = (projections[z0].px);
        z1px = (projections[z1].px);
        z0dx = ((projections[z0].dx));
        z1dx = ((projections[z1].dx));
        
        px0z0 = z0px - (x0 * z0dx);
        px1z1 = z1px - ((dX + x0) * z1dx);
        
        z1py = (projections[z1].py);
        z0py = (projections[z0].py);
        
        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);
        
        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
        
    } else {
        z0px = (projections[z1].px);
        z1px = (projections[z0].px);
        z0dx = ((projections[z1].dx));
        z1dx = ((projections[z0].dx));
        
        px0z0 = z0px - ((x0) * z0dx);
        px1z1 = z1px - ((x0 + dX) * z1dx); //extra operations to avoid overflow
        
        z1py = (projections[z0].py);
        z0py = (projections[z1].py);
        
        py0z0 = z0py + ((y0) * z0dx);
        py0z1 = z1py + ((y0) * z1dx);
        
        py1z0 = z0py + ((y0 + dY) * z0dx);
        py1z1 = z1py + ((y0 + dY) * z1dx);
    }
    
    if (px1z1 < 0 || px0z0 > 63) {
        return 0;
    }
    
#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);
    
    fix_line( px0z1, py0z1, px1z1, py0z1, 4);
    
    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif
    
    {
        int16_t x0, x1;
        
        
        if (elementMask & 2) {
            if (IN_RANGE(0, 63, px0z0) && stencilHigh[px0z0] < py0z0) {
                vLine(px0z0, py0z0, max(py1z0, stencilHigh[px0z0]));
            }
        }
        
        if (elementMask & 1) {
            if (IN_RANGE(0, 63, px1z1) && py0z1 > stencilHigh[px1z1]) {
                vLine(px1z1, py0z1, max(py1z1, stencilHigh[px1z1]));
            }
        }
        
        
        /* The upper segment */
        x0 = px0z0;
        x1 = px1z1;
        
        if (x0 != x1) {
            int16_t upperY0 = py1z0;
            int16_t upperY1 = py1z1;
            int16_t upperDx = abs(x1 - x0);
            int16_t upperSx = x0 < x1 ? 1 : -1;
            int16_t upperDy = -abs(upperY1 - upperY0);
            int16_t upperSy = upperY0 < upperY1 ? 1 : -1;
            int16_t upperErr = upperDx + upperDy;  /* error value e_xy */
            int16_t upperErr2;
            int16_t lowerY0 = py0z0;
            int16_t lowerY1 = py0z1;
            int16_t lowerDx = abs(x1 - x0);
            int16_t lowerSx = x0 < x1 ? 1 : -1;
            int16_t lowerDy = -abs(lowerY1 - lowerY0);
            int16_t lowerSy = lowerY0 < lowerY1 ? 1 : -1;
            int16_t lowerErr = lowerDx + lowerDy;  /* error value e_xy */
            int16_t lowerErr2 = 0;
            
            while ((x0 != x1 && (upperY0 != upperY1 || lowerY0 != lowerY1))) {
                
                if (IN_RANGE(0, 63, x0)) {
                    if (stencilHigh[x0] <= upperY0) {
                        buffer[(upperY0 * 64) + x0] |= 1;
                    }
                    
                    if (stencilHigh[x0] < lowerY0) {
                        stencilHigh[x0] = lowerY0;
                    }
                }
                
                /* loop */
                upperErr2 = upperErr * 2;
                
                if (upperErr2 >= upperDy || lowerErr2 >= lowerDy) {
                    upperErr += upperDy; /* e_xy+e_x > 0 */
                    lowerErr += lowerDy; /* e_xy+e_x > 0 */
                    x0 += lowerSx;
                }
                
                if (x0 >= 64) {
                    return 0;
                }
                
                if (upperErr2 <= upperDx) {
                    /* e_xy+e_y < 0 */
                    upperErr += upperDx;
                    upperY0 += upperSy;
                }
                
                /* loop */
                lowerErr2 = lowerErr * 2;
                
                if (lowerErr2 <= lowerDx) {
                    /* e_xy+e_y < 0 */
                    lowerErr += lowerDx;
                    lowerY0 += lowerSy;
                }
            }
        }
    }
    
    return 1;
}

uint8_t drawCubeAt(int8_t x0, int8_t y0, int8_t z0, int8_t dX, int8_t dY, int8_t dZ, uint8_t elementMask) {
    
    int8_t z1;
    uint8_t z0px;
    uint8_t z0py;
    uint8_t z1px;
    uint8_t z1py;
    int8_t z0dx;
    int8_t z1dx;
    
    int16_t px0z0;
    int8_t py0z0;
    int16_t px1z0;
    int8_t py1z0;
    int16_t px0z1;
    int8_t py0z1;
    int16_t px1z1;
    
    uint8_t drawContour;
    
    if (z0 >= 32) {
        return 0;
    }
    
    z1 = z0 + dZ;
    
    if (z1 >= 32) {
        return 0;
    }
    
    
    z0px = (projections[z0].px);
    z1px = (projections[z1].px);
    z0dx = ((projections[z0].dx));
    z1dx = ((projections[z1].dx));
    
    px0z0 = z0px - ((x0) * z0dx);
    px0z1 = z1px - ((x0) * z1dx);
    
    px1z0 = px0z0 - (dX * z0dx);
    px1z1 = px0z1 - (dX * z1dx);
    
    z1py = (projections[z1].py);
    z0py = (projections[z0].py);
    
    py0z0 = z0py + ((y0) * z0dx);
    py1z0 = py0z0 + (dY * z0dx);
    py0z1 = z1py + ((y0) * z1dx);
    
    if (px1z0 < 0 || px0z0 > 63) {
        return 0;
    }
    
    drawContour = (dY);
    
#ifdef DEBUG_WIREFRAME
    fix_line( px0z0, py0z0, px1z0, py0z0, 4);
    fix_line( px0z0, py0z0, px0z0, py1z0, 4);
    fix_line( px1z0, py0z0, px1z0, py1z0, 4);
    fix_line( px0z0, py1z0, px1z0, py1z0, 4);
    
    fix_line( px0z1, py0z1, px1z1, py0z1, 4);
    
    fix_line( px0z0, py0z0, px0z1, py0z1, 4);
    fix_line( px1z0, py0z0, px1z1, py0z1, 4);
    return;
#endif
    
    {
        int16_t x, x0, x1;
        
        if (drawContour) {
            if (elementMask & 2) {
                if (IN_RANGE(0, 63, px0z0) && stencilHigh[px0z0] < py0z0) {
                    vLine(px0z0, py0z0, stencilHigh[px0z0]);
                }
                
                if (IN_RANGE(0, 63, px1z0) && stencilHigh[px1z0] < py0z0) {
                    vLine(px1z0, py0z0, stencilHigh[px1z0]);
                }
            }
            
            if (elementMask & 1) {
                if (IN_RANGE(0, 63, px0z1) && px0z1 < px0z0 && py0z1 > stencilHigh[px0z1]) {
                    vLine(px0z1, py0z1, stencilHigh[px0z1]);
                }
                
                if (IN_RANGE(0, 63, px1z1) && px1z1 > px1z0 && py0z1 > stencilHigh[px1z1]) {
                    vLine(px1z1, py0z1, stencilHigh[px1z1]);
                }
            }
        }
        
        /* Draw the horizontal outlines of z0 and z1 */
        
        if (py0z0 > py0z1) {
            /* Ceiling is lower than camera */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z0) {
                    if (drawContour) {
                        buffer[(stencilHigh[x] * 64) + x] |= 1;
                    }
                    stencilHigh[x] = py0z0;
                }
            }
        } else if (drawContour) {
            /* Ceiling is higher than the camera*/
            /* Let's just draw the nearer segment */
            for (x = px0z0; x <= px1z0; ++x) {
                if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z0) {
                    buffer[(stencilHigh[x] * 64) + x] |= 1;
                }
            }
        }
        
        
        /* The left segment */
        x0 = px0z0;
        x1 = px0z1;
        
        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;
            
            while ((x0 != x1 || y0 != y1)) {
                
                if (IN_RANGE(0, 63, x0)) {
                    if (stencilHigh[x0] < y0) {
                        if (drawContour) {
                            buffer[(stencilHigh[x0] * 64) + x0] |= 1;
                        }
                        stencilHigh[x0] = y0;
                    }
                }
                
                /* loop */
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }
                
                if (x0 >= 64) {
                    goto right_stroke;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }
        
    right_stroke:
        
        /* The right segment */
        x0 = px1z0;
        x1 = px1z1;
        
        if (x0 != x1) {
            int16_t y0 = py0z0;
            int16_t y1 = py0z1;
            int16_t dx = abs(x1 - x0);
            int16_t sx = x0 < x1 ? 1 : -1;
            int16_t dy = -abs(y1 - y0);
            int16_t sy = y0 < y1 ? 1 : -1;
            int16_t err = dx + dy;  /* error value e_xy */
            int16_t e2;
            
            while ((x0 != x1 || y0 != y1)) {
                
                if (IN_RANGE(0, 63, x0) && stencilHigh[x0] < y0) {
                    if (drawContour) {
                        buffer[(stencilHigh[x0] * 64) + x0] |= 1;
                    }
                    stencilHigh[x0] = y0;
                }
                
                /* loop */
                e2 = err << 2;
                
                if (e2 >= dy) {
                    err += dy; /* e_xy+e_x > 0 */
                    x0 += sx;
                }
                
                if (x0 >= 64) {
                    goto final_stroke;
                }
                
                if (e2 <= dx) {
                    /* e_xy+e_y < 0 */
                    err += dx;
                    y0 += sy;
                }
            }
        }
        
    final_stroke:
        if (py0z0 <= py0z1) {
            /* Ceiling is higher than the camera*/
            /* Draw the last segment */
            
            if (drawContour) {
                for (x = px0z1; x <= px1z1; ++x) {
                    if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z1) {
                        stencilHigh[x] = py0z1;
                    }
                }
            } else {
                for (x = px0z1; x <= px1z1; ++x) {
                    if (IN_RANGE(0, 63, x) && stencilHigh[x] < py0z1) {
                        stencilHigh[x] = py0z1;
                    }
                }
            }
        }
    }
    
    return 1;
}


uint8_t drawPattern(uint8_t pattern, uint8_t x0, uint8_t x1, uint8_t y) {
    
    int8_t diff = patterns[0].ceiling - patterns[pattern].ceiling;
    uint8_t type = patterns[pattern].geometryType;
    
    if (patterns[pattern].block) {
        return 0;
    }
    
    if (type == 0) {
        return drawCubeAt(x0, patterns[pattern].ceiling - 1, y, x1 - x0,
                          diff, 1, patterns[pattern].elementsMask);
        
    } else {
        switch (cameraRotation) {
            case DIRECTION_W:
            case DIRECTION_E:
                if (type == WEDGE_TYPE_NEAR_LEFT) {
                    type = WEDGE_TYPE_NEAR_RIGHT;
                } else {
                    type = WEDGE_TYPE_NEAR_LEFT;
                }
                break;
                
        }
        
        return drawWedge(x0, patterns[pattern].ceiling - 1, y, x1 - x0,
                         diff, 1, patterns[pattern].elementsMask, type);
    }
}

void renderScene() {
    uint8_t lastPattern, lastIndex;
    
    switch (cameraRotation) {
        case DIRECTION_N: {
            int8_t y;
            int8_t limit = max(cameraZ - 19, 0);
            for (y = min(cameraZ - 3, 31); y >= limit; --y) {
                int8_t x;
                int8_t const *mapY = &map[y][0];
                int8_t const *mapXY;
                int8_t minX = cameraX + 5 + ((cameraZ - 3) - y);
                int8_t maxX = 0;
                
                if (minX > (cameraX + 16)) {
                    minX = cameraX + 16;
                }
                
                lastIndex = cameraX;
                lastPattern = *(mapY + lastIndex);
                
                mapXY = &map[y][lastIndex];
                for (x = lastIndex; x < minX - 1; ++x) {
                    uint8_t pattern;
                    
                    pattern = *mapXY;
                    
                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            if (!drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y)) {
                                x = minX - 1;
                            }
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }
                    
                    ++mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, lastIndex - cameraX, x - cameraX, cameraZ - y);
                }
                
                lastIndex = cameraX - 1;
                
                if (!cameraX) {
                    lastIndex = 0;
                }
                
                lastPattern = *(mapY + lastIndex);
                
                mapXY = &map[y][lastIndex];
                
                maxX = max(cameraX - 3 - ((cameraZ - 3) - y), 0);
                
                for (x = lastIndex; x >= maxX + 1; --x) {
                    uint8_t pattern;
                    pattern = *mapXY;
                    
                    if (pattern != lastPattern) {
                        if (lastPattern != 0) {
                            
                            if (!drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y)) {
                                x = maxX + 1;
                            }
                            
                            lastIndex = x;
                        }
                        lastPattern = pattern;
                    }
                    
                    --mapXY;
                }
                if (lastPattern != 0) {
                    drawPattern(lastPattern, x + 1 - cameraX, lastIndex + 1 - cameraX, cameraZ - y);
                }
            }
        }
            break;
            
        case DIRECTION_E: {
            int8_t x;
            for (x = min(cameraX - 3, 31); x <= min(cameraX + 13, 31); ++x) {
                int8_t y;
                
                for (y = cameraZ; y <= min(cameraZ + (x - cameraX), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }
                
                for (y = max(cameraZ - 1, 0); y >= max(cameraZ - (x - cameraX), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, x - cameraX + 3);
                }
                
            }
        }
            break;
            
        case DIRECTION_S: {
            int8_t y;
            for (y = min(cameraZ + 3, 31); y <= min(cameraZ + 19, 31); ++y) {
                int8_t x;
                for (x = cameraX; x <= min(cameraX + (y - (cameraZ + 3)), 31); ++x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
                
                for (x = max(cameraX - 1, 0); x >= max(cameraX - (y - (cameraZ + 3)), 0); --x) {
                    drawPattern(map[y][x], cameraX - x, cameraX - x + 1, y - cameraZ);
                }
            }
        }
            break;
            
        case DIRECTION_W: {
            int8_t x;
            for (x = max(cameraX, 0); x >= max(cameraX - 16, 0); --x) {
                int8_t y;
                for (y = cameraZ; y <= min(cameraZ - (x - (cameraX)), 31); ++y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
                
                for (y = max(cameraZ - 1, 0); y >= max(cameraZ + (x - (cameraX)), 0); --y) {
                    drawPattern(map[y][x], y - cameraZ + 3, y + 1 - cameraZ + 3, cameraX - x + 1);
                }
            }
        }
            break;
    }
    
    for (uint8_t x = 0; x < 64; ++x) {
        buffer[(stencilHigh[x] * 64) + x] |= 1;
    }
}

void tickRenderer() {
    uint8_t prevX;
    uint8_t prevZ;
    
    
    renderScene();
    
    
    
    graphicsFlush();
    memset(stencilHigh, 0, 64);
    
    prevX = cameraX;
    prevZ = cameraZ;
    
waitkey:
    switch (getKey()) {
        case 'q':
            cameraRotation--;
            if (cameraRotation < 0) {
                cameraRotation = 3;
            }
            break;
        case 'e':
            cameraRotation = (cameraRotation + 1) & 3;
            break;
        case 'l':
            running = 0;
            break;
            
        case 'a':
            cameraX -= 2;
            break;
        case 'd':
            cameraX += 2;
            break;
            
            
        case 's':
            switch (cameraRotation) {
                case 0:
                    cameraZ += 2;
                    break;
                case 1:
                    cameraX -= 2;
                    break;
                case 2:
                    cameraZ -= 2;
                    break;
                case 3:
                    cameraX += 2;
                    break;
            }
            
            
            break;
        case 'w':
            switch (cameraRotation) {
                case 0:
                    cameraZ -= 2;
                    break;
                case 1:
                    cameraX += 2;
                    break;
                case 2:
                    cameraZ += 2;
                    break;
                case 3:
                    cameraX -= 2;
                    break;
            }
            break;
            
            
        default:
            goto waitkey;
    }
    
    if (cameraZ >= 32) {
        cameraZ = 31;
    }
    
    if (cameraX >= 32) {
        cameraX = 31;
    }
    
    if (cameraZ < 0) {
        cameraZ = 0;
    }
    
    if (cameraX < 0) {
        cameraX = 0;
    }
    
    if (patterns[map[cameraZ - 2][cameraX]].ceiling < 2) {
        cameraX = prevX;
        cameraZ = prevZ;
    }
}


int demoMain() {
    running = 1;
    cameraX = 5;
    cameraZ = 15;
    cameraRotation = 0;
    init();
    
    memset(stencilHigh, 0, 64);
    
    do {
        tickRenderer();
    } while (running);
    
    shutdownGraphics();
    
    return 0;
}


#ifdef __MAIN__

void main() {
    demoMain();
}
#endif

