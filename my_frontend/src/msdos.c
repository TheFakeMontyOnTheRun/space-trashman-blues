#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"

unsigned char imageBuffer[128 * 32];

enum ESoundDriver soundDriver = kPcSpeaker;
extern uint8_t firstFrameOnCurrentState;
extern enum EGameMenuState currentGameMenuState;

#ifdef SUPPORTS_ROOM_TRANSITION_ANIMATION
extern uint8_t roomTransitionAnimationStep;
#endif

extern enum EDirection playerDirection;
extern int8_t cameraX;
extern int8_t cameraZ;

char playerPositionSprite[4][8]={
        {
                0b00011000,
                0b00111100,
                0b01111110,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000
        },
        {
                0b00100000,
                0b01100000,
                0b11100000,
                0b11100000,
                0b01100000,
                0b00100000,
                0b00000000,
                0b00000000
        },
        {
                0b01111110,
                0b00111100,
                0b00011000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000,
                0b00000000
        },
        {
                0b00000100,
                0b00000110,
                0b00000111,
                0b00000111,
                0b00000110,
                0b00000100,
                0b00000000,
                0b00000000
        },
};

void put_sprite_8(uint16_t x, uint8_t y, uint8_t *sprite, uint8_t colour) {

    for(uint8_t c = 0; c < 8; ++c) {
        uint8_t line = *sprite;
        for (uint16_t d = 0; d < 8; ++d) {
            if (line & 1) {
                realPut( x + d, y + c, colour, NULL);
            }
            line = line >> 1;
        }
        ++sprite;
    }
}

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

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    int x;

    for (x = 0; x < XRESMINUSONE;) {
        uint8_t y, prevY, c;
        uint8_t *ptr;
        next_cluster:
        /* pixel 1 */
        y = *stencilPtr;
        prevY = y;
        ptr = &imageBuffer[(32 * (y & 0b01111111)) + ((x & 0b01111111) / 4)];
        uint8_t currByte = *ptr;

        switch (x & 3) {
            case 3:
                currByte = (currByte & 0b11111100) | 0b00000001;
                break;
            case 2:
                currByte = (currByte & 0b11110011) | 0b00000100;
                break;
            case 1:
                currByte = (currByte & 0b11001111) | 0b00010000;
                break;
            case 0:
                currByte = (currByte & 0b00111111) | 0b01000000;
                break;
        }

        if (x & 3) {
            ++x;
            ++stencilPtr;
            *ptr = currByte;
            continue;
        }

        for (c = 2; c < 4; ++c) {
            ++x;
            ++stencilPtr;
            y = *stencilPtr;
            if (y != prevY) {
                *ptr = currByte;
                goto next_cluster;
            }

            switch (x & 3) {
                case 3:
                    currByte = (currByte & 0b11111100) | 0b00000001;
                    break;
                case 2:
                    currByte = (currByte & 0b11110011) | 0b00000100;
                    break;
                case 1:
                    currByte = (currByte & 0b11001111) | 0b00010000;
                    break;
                case 0:
                    currByte = (currByte & 0b00111111) | 0b01000000;
                    break;
            }
        }
        *ptr = currByte;

        ++x;
        ++stencilPtr;
    }
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
    memFill(imageBuffer, 0, 128 * 32);
}

void initHW(int argc, char **argv) {
    asm volatile(
            "movb $0x0, %%ah\n\t"
            "movb $0x4, %%al\n\t"
            "int $0x10\n\t"
            :
            :
            : "ax"
            );

    initKeyboardUI();
    clearGraphics();
}

void clearScreen(void) {
    asm volatile("movb $0x0, %%ah\n\t"
                 "movb $0x4, %%al\n\t"
                 "int $0x10\n\t"
            :
            :
            : "ax"
            );
}

void handleSystemEvents(void) {}

enum ECommand getInput(void) {
    unsigned char toReturn = 255;

    asm volatile ("movb $0x01, %%ah\n\t"
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

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                0
        );
    }

    if (waitForKey) {
        if (toReturn == '2') {
            waitForKey = 0;
            firstFrameOnCurrentState = 1;
            needsToRedrawVisibleMeshes = 1;
            return kCommandNone;
        }

        return kCommandNone;
    }

    performAction();

    switch(toReturn) {
        case 'q':
            return kCommandLeft;
        case 'w':
            return kCommandUp;
        case 's':
            return kCommandDown;
        case 'e':
            return kCommandRight;
        case 'z':
            return kCommandStrafeLeft;
        case 'x':
            return kCommandStrafeRight;
        case 'l':
            return kCommandBack;
        case '1':
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

void drawTextAtWithMarginWithFiltering(const int _x, const int _y, int limitX, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {

    const char *ptr = text;
    uint16_t c = 0;
    uint16_t chary = 0;
    uint16_t x = _x;
    uint8_t y = _y;
    char cha = *ptr;

    for (; cha && y < 25; ++c) {

        if (cha == '-') {
            cha = charToReplaceHifenWith;
        }

        if (x >= (limitX / 8) || x >= (XRES_TEXT)) {
            ++y;
            x = _x + 1;
        } else if (cha == '\n') {
            ++y;
            x = _x + 1;
            ++ptr;
            cha = *ptr;
            continue;
        } else {
            ++x;
        }

        uint8_t biosX = x - 1;

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
                : "rm" (biosX), "rm" (y), "rm"(cha)
                : "ax", "bx", "cx", "dx"
                );

        ++ptr;
        cha = *ptr;
    }
}

void startFrame(int x, int y, int width, int height) {

}

void endFrame(void) {

    uint16_t baseOffset = 0;
    uint16_t index = 0;

    if (!needsToRedrawVisibleMeshes) {
        return;
    }

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                1
        );
    }

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

    memFill(imageBuffer, 0, 128 * 32);
}


void enterTextMode(void) {}

void exitTextMode(void) {
    clearScreen();
}


void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour, NULL);
        }
    }
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
