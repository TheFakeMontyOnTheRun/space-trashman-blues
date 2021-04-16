
/*
This demo draws pixels and lines in Mode 2.
Note that when lines of two different colors overlap,
they create "clashing" effects.
*/

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

#define PATTERN		((const cv_vmemp)0x0000)
#define IMAGE		((const cv_vmemp)0x1800)
#define COLOR		((const cv_vmemp)0x2000)
#define SPRITE_PATTERNS ((const cv_vmemp)0x3800)
#define SPRITES		((const cv_vmemp)0x3c00)

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

uint8_t buffer[(64 * 64)];

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



void commitPixels() {
  uint8_t x, y, _x, _y;
  byte b;
  word ofs, _yoff;
  uint8_t *ptr = &buffer[0];
  uint8_t pixel;
  _y = 128;
  
  for (y = 64; y; --y, _y -= 2 ) {
    
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
                          b |= 128 >> ((_x+1)&7);   
                }

          } else {
                if ( (pixel & 2) ) {
                          b &= ~(128 >> ( _x     &7));                     
                          b &= ~(128 >> ((_x + 1)&7));
                }
           }        
           *ptr = pixel << 1;
           ptr++;
	   _x += 2;
      
      //2
	pixel = *ptr;
        if ( pixel & 1 ) {
		if ( ~(pixel & 2) ) {
                          b |= 128 >> (_x & 7);
                          b |= 128 >> ((_x+1)&7);
                }
          } else {
                if ( (pixel & 2) ) {
                          b &= ~(128 >> ( _x     &7));                     
                          b &= ~(128 >> ((_x + 1)&7));
                }
           }        
           *ptr = pixel << 1;
           ptr++;    
	   _x += 2;      
      
      //3
	pixel = *ptr;
        if ( pixel & 1 ) {
		if ( ~(pixel & 2) ) {
                          b |= 128 >> (_x & 7);
                          b |= 128 >> ((_x+1)&7);
                }

          } else {
                if ( (pixel & 2) ) {
                          b &= ~(128 >> ( _x     &7));                     
                          b &= ~(128 >> ((_x + 1)&7));
                }
           }        
           *ptr = pixel << 1;
           ptr++;
	   _x += 2;
      
      //4
	pixel = *ptr;
        if ( pixel & 1 ) {
		if ( ~(pixel & 2) ) {
                          b |= 128 >> (_x & 7);
                          b |= 128 >> ((_x+1)&7);
                }
          } else {
                if ( (pixel & 2) ) {
                          b &= ~(128 >> ( _x     &7));                     
                          b &= ~(128 >> ((_x + 1)&7));
                }
           }        
           *ptr = pixel << 1;
           ptr++;    
	   _x += 2;
      
	  x += 4;
	  cvu_voutb(b, PATTERN + ofs);
	  cvu_voutb(32, COLOR + ofs);    

      
        }
    }
}



void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
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

#ifdef __MAIN__

void main() {
  uint8_t counter;
      uint8_t newX, newY;
  setup_mode2();  
  cv_set_screen_active(true);

    memset(buffer, 0, 64 * 64);  

  while(1) {
    
    uint8_t lastX, lastY;
          counter = 16;


  
   draw_line(0, 0, 63, 0);
    draw_line(0, 0, 0, 63);
     draw_line(0, 63, 63, 63);
      draw_line(63, 0, 63, 63);
  	lastX = rand() & 63;
	lastY = rand() & 63; 
    
    while(counter--) {

      


      newX = rand() & 63;
      newY = rand() & 63;
      draw_line(lastX, lastY, newX, newY);
      
      lastX = newX;
      lastY = newY;
      


    }
	commitPixels();    
    
  }
}

#endif


