#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

#include "KeyboardUI.h"

unsigned char imageBuffer[128 * 32];

uint8_t updateDirection;

void shutdownGraphics(void) {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {

    if (y0 > y1) {
        uint8_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }

    uint16_t dy = (y1 - y0);
    uint8_t *ptr = &imageBuffer[(32 * y0) + (x0 / 4)];

    switch (x0 & 3) {
        case 3: {
            if (shouldStipple) {
                while (dy--) {
                    if (dy & 1) {
                        *ptr = (*ptr & 0b11111100) | 1;
                    }
                    ptr += 32;
                }
            } else {
                while (dy--) {
                    *ptr = (*ptr & 0b11111100) | 1;
                    ptr += 32;
                }
            }
        }
            break;
        case 2:
            if (shouldStipple) {
                while (dy--) {
                    if (dy & 1) {
                        *ptr = (*ptr & 0b11110011) | 4;
                    }
                    ptr += 32;
                }
            } else {
                while (dy--) {
                    *ptr = (*ptr & 0b11110011) | 4;
                    ptr += 32;
                }
            }
            break;
        case 1:
            if (shouldStipple) {
                while (dy--) {
                    if (dy & 1) {
                        *ptr = (*ptr & 0b11001111) | 16;
                    }

                    ptr += 32;
                }
            } else {
                while (dy--) {
                    *ptr = (*ptr & 0b11001111) | 16;
                    ptr += 32;
                }
            }
            break;
        case 0:
            if (shouldStipple) {
                while (dy--) {
                    if (dy & 1) {
                        *ptr = (*ptr & 0b00111111) | 64;
                    }
                    ptr += 32;
                }
            } else {
                while (dy--) {
                    *ptr = (*ptr & 0b00111111) | 64;
                    ptr += 32;
                }
            }
            break;
    }
}

uint8_t *graphicsPutAddr(uint8_t x, uint8_t y, uint8_t colour, uint8_t *ptrToByte) {

    if (ptrToByte == NULL) {
        ptrToByte = &imageBuffer[(32 * (y & 0b01111111)) + ((x & 0b01111111) / 4)];
    }

    switch (x & 3) {
        case 3:
            *ptrToByte = (*ptrToByte & 0b11111100) | 0b00000001;
            break;
        case 2:
            *ptrToByte = (*ptrToByte & 0b11110011) | 0b00000100;
            break;
        case 1:
            *ptrToByte = (*ptrToByte & 0b11001111) | 0b00010000;
            break;
        case 0:
            *ptrToByte = (*ptrToByte & 0b00111111) | 0b01000000;
            break;
    }

    return ptrToByte;
}

void graphicsPut(uint8_t x, uint8_t y) {
    uint8_t *ptrToByte = &imageBuffer[(32 * (y & 0b01111111)) + ((x & 0b01111111) / 4)];

    switch (x & 3) {
        case 3:
            *ptrToByte = (*ptrToByte & 0b11111100) | 0b00000001;
            break;
        case 2:
            *ptrToByte = (*ptrToByte & 0b11110011) | 0b00000100;
            break;
        case 1:
            *ptrToByte = (*ptrToByte & 0b11001111) | 0b00010000;
            break;
        case 0:
            *ptrToByte = (*ptrToByte & 0b00111111) | 0b01000000;
            break;
    }
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t value, uint8_t *ptr) {

    int pixelRead = 0;
    uint16_t offset = ((x / 4) + ((y / 2) * 80));

    if (y & 1) {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %1, %%di  \n\t"
                     "xorw %%ax, %%ax\n\t"
                     "movb %%es:(%%di), %%al\n\t"
                     "movw %%ax, %0\n\t"
                : "=r"(pixelRead)
                : "r"( 0x2000 + offset )
                : "ax", "es", "di"
                );
    } else {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %1, %%di  \n\t"
                     "xorw %%ax, %%ax\n\t"
                     "movb %%es:(%%di), %%al\n\t"
                     "movw %%ax, %0\n\t"
                : "=r"(pixelRead)
                : "r"(offset)
                : "ax", "es", "di"
                );
    }

    uint8_t pixel = pixelRead & 0xFFFF;

    switch (x & 3) {
        case 3:
            pixel = value | (pixel & 0b11111100);
            break;
        case 2:
            value = (value << 2);
            pixel = value | (pixel & 0b11110011);
            break;

        case 1:
            value = (value << 4);
            pixel = value | (pixel & 0b11001111);
            break;

        case 0:
            value = (value << 6);
            pixel = value | (pixel & 0b00111111);
            break;
    }

    value = pixel;

    if (y & 1) {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %0, %%di  \n\t"
                     "movb %1, %%es:(%%di)\n\t"
                :
                : "r"( 0x2000 + offset), "r" (value)
                : "ax", "es", "di"
                );
    } else {
        asm volatile("movw $0xb800, %%ax\n\t"
                     "movw %%ax, %%es\n\t"
                     "movw %0, %%di  \n\t"
                     "movb %1, %%es:(%%di)\n\t"
                :
                : "r"(offset), "r" (value)
                : "ax", "es", "di"
                );
    }

    return NULL;
}

void clearGraphics(void) {
    memset(imageBuffer, 0, 128 * 32);
}

void init(void) {
    initKeyboardUI();
    updateDirection = 1;
    asm volatile("movb $0x0, %%ah\n\t"
                 "movb $0x4, %%al\n\t"
                 "int $0x10\n\t"
            :
            :
            : "ax"
            );
}

void clearScreen(void) {
    init();
}

uint8_t getKey(void) {
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

    if (toReturn == 'q' || toReturn == 'e') {
        updateDirection = 1;
    }

    return toReturn;
}

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {

    const char *ptr = text;
    uint16_t c = 0;
    uint16_t chary = 0;
    uint16_t x = _x - 1;
    char cha = *ptr;

    for (; cha && y < 25; ++c) {

        if (x == limitX) {
            ++y;
            x = _x - 1;
        } else if (cha == '\n') {
            ++y;
            x = _x - 1;
            ++ptr;
            cha = *ptr;
            continue;
        } else {
            ++x;
        }

        asm volatile (
                "movb $0x02, %%ah\n\t"
                "movb    %0, %%dl\n\t"
                "movb    %1, %%dh\n\t"
                "movb  $0x0, %%bh\n\t"
                "int  $0x10\n\t"
                "movb $0x09, %%ah\n"
                "movb %2,    %%al\n"
                "movw $0x01, %%cx\n"
                "movb $0x0,  %%bh\n"
                "movb $0x03, %%bl\n"
                "int  $0x10\n\t"
                :
                : "rm" (x), "rm" (y), "rm"(cha)
                : "ax", "bx", "cx", "dx"
                );

        ++ptr;
        cha = *ptr;
    }
}


void graphicsFlush(void) {
    uint16_t baseOffset = 0;
    uint16_t index = 0;

/*
 * DS ES and SS already have the same value as CS
 * */

    asm volatile(
        /* set ES to point to VRAM */
            "movw $0xb800, %%ax\n\t"
            "movw %%ax, %%es\n\t"

            /* clear direction flag */
            "cld\n\t"
            :
            :
            : "ax", "es"
            );

    /* 4096 = 128 * 32 */
    for (index = 0; index < 4096; index += 64, baseOffset += 80) {
        asm volatile(
            /* point to the correct offset inside VRAM */
                "movw %0, %%di\n\t"

                /* point SI to imageBuffer */
                "movw %1, %%ax\n\t"
                "addw $imageBuffer, %%ax\n\t"
                "movw %%ax, %%si\n\t"

                /* we will copy 32-bytes */
                "movw $16, %%cx\n\t"

                /* copy the damn thing
                   DS:[SI] to ES:[DI], CX times */
                "rep movsw\n\t"

                /* Second line
                   we will copy 32-bytes */
                "movw $16, %%cx\n\t"

                /* 0x2000 = 8192
                   8192 - 32 = 8160 */
                "addw $8160, %%di\n\t"

                /* point SI to imageBuffer */
                "addw $32, %%ax\n\t"
                "movw %%ax, %%si\n\t"

                /* copy the damn thing
                   DS:[SI] to ES:[DI], CX times */
                "rep movsw\n\t"
                :
                : "r"( baseOffset ), "r"(index)
                : "ax", "cx", "es", "di", "si"
                );
    }

    if (updateDirection) {
        updateDirection = 0;
        switch (getPlayerDirection()) {
            case 0:
                writeStrWithLimit(12, 18, "N", 31, 2, 0);
                break;
            case 1:
                writeStrWithLimit(12, 18, "E", 31, 2, 0);
                break;
            case 2:
                writeStrWithLimit(12, 18, "S", 31, 2, 0);
                break;
            case 3:
                writeStrWithLimit(12, 18, "W", 31, 2, 0);
                break;
        }
    }

    memset(imageBuffer, 0, 128 * 32);
}


void clearTextScreen(void) {
    clearScreen();
}

void enterTextMode(void) {}

void exitTextMode(void) {
    clearScreen();
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