/*
   Created by Daniel Monteiro on 11/07/2023.
*/

#ifndef DERELICT8_FONT_H
#define DERELICT8_FONT_H

/* Adding const here actually doubles the ROM size! */
static uint8_t font[] = {
        /*  ASCII table starting on SPACE. */
        /*  Being on line 32 is no accident. */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 /*  space */
        , 0x10, 0x38, 0x38, 0x10, 0x10, 0x00, 0x10, 0x00, 0x6c, 0x6c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28,
        0x7c, 0x28, 0x28, 0x7c, 0x28, 0x00, 0x20, 0x38, 0x40, 0x30, 0x08, 0x70, 0x10, 0x00, 0x64, 0x64, 0x08, 0x10,
        0x20, 0x4c, 0x4c, 0x00, 0x20, 0x50, 0x50, 0x20, 0x54, 0x48, 0x34, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x20, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x00,
        0x00, 0x28, 0x38, 0x7c, 0x38, 0x28, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x30, 0x30, 0x00, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00 /*  space - 15 */
        , 0x38, 0x44, 0x4c, 0x54, 0x64, 0x44, 0x38, 0x00 /*  0 */
        , 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x38, 0x44, 0x04, 0x18, 0x20, 0x40, 0x7c, 0x00, 0x38, 0x44,
        0x04, 0x38, 0x04, 0x44, 0x38, 0x00, 0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x7c, 0x40, 0x40, 0x78,
        0x04, 0x44, 0x38, 0x00, 0x18, 0x20, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x7c, 0x04, 0x08, 0x10, 0x20, 0x20,
        0x20, 0x00, 0x38, 0x44, 0x44, 0x38, 0x44, 0x44, 0x38, 0x00, 0x38, 0x44, 0x44, 0x3c, 0x04, 0x08, 0x30, 0x00,
        0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x20, 0x08, 0x10,
        0x20, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x10, 0x08, 0x04,
        0x08, 0x10, 0x20, 0x00, 0x38, 0x44, 0x04, 0x18, 0x10, 0x00, 0x10, 0x00, 0x38, 0x44, 0x5c, 0x54, 0x5c, 0x40,
        0x38, 0x00 /*  0 */
        , 0x38, 0x44, 0x44, 0x44, 0x7c, 0x44, 0x44, 0x00 /*  a */
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

#endif /* DERELICT8_FONT_H */
