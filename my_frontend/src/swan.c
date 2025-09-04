#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <ws.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "GamepadUI.h"
#include "font.h"

// The way data is stored is a little peculiar, to work with the Swan's 2bpp tiles.
// Imagine a 32x24 (4x3 tile) display:
//
// AaGg
// BbHh
// CcIi
//
// It is built from six 2bpp tiles. The first column uses the first plane of the
// first three tiles; the second column uses the second plane of the first three
// tiles; the next two columns use the next tile, and so forth.

#define TILE_OFFSET 256
#define TILE_COUNT (WS_DISPLAY_WIDTH_TILES * WS_DISPLAY_HEIGHT_TILES / 2)

__attribute__((section(".iramx_2bpp_3000")))
uint8_t tileData[WS_DISPLAY_TILE_SIZE * TILE_COUNT];

// The screen never scrolls, so it is placed in a memory area which maximizes
// contiguity (the lower-right 28x18 tiles of a 2048-byte screen block).

__attribute__((section(".iramx_2b88")))
uint16_t screenData[32 * 18 - 4];
#define SCREEN_BASE 5

#define BUFFER_WIDTH 128
#define BUFFER_HEIGHT 128
unsigned char imageBuffer[BUFFER_WIDTH * BUFFER_HEIGHT / 8];

#define COOLDOWN_MAX 0x1F
uint8_t cooldown;

enum ESoundDriver soundDriver = kNoSound;
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

void vLine(uint8_t x, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
    uint8_t _y0 = y0;
    uint8_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (shouldStipple && ((_y0 & 1) == 0)) {
        ++_y0;
    }

    uint8_t *ptr = &imageBuffer[(_y0 << 1) + ((x & 8) >> 3) + ((x & 240) * 16)];
    uint8_t v = 128 >> (x & 7);

    if (shouldStipple) {
        for (uint8_t y = _y0; y <= _y1; y += 2, ptr += 4)
            *ptr |= v;
    } else {
        for (uint8_t y = _y0; y <= _y1; y++, ptr += 2)
            *ptr |= v;
    }
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    uint16_t x;

    for (x = 0; x < XRESMINUSONE; ) {
        uint8_t y = *stencilPtr;

        graphicsPut(x, y);

        ++x;
        ++stencilPtr;
    }
}

void graphicsPut(uint8_t x, uint8_t y) {
    imageBuffer[(y << 1) + ((x & 8) >> 3) + ((x & 240) * 16)] |= (128 >> (x & 7));
}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    if (ptr == NULL) {
        ptr = &tileData[(y << 1) + ((x & 8) >> 3) + ((x & 240) * WS_DISPLAY_HEIGHT_TILES)];
    }

    uint8_t v = 128 >> (x & 7);

    if (colour)
        *ptr |= v;
    else
        *ptr &= ~v;

    return ptr;
}

void clearGraphics(void) {
    memFill(imageBuffer, 0, sizeof(imageBuffer));
}

void initHW(int argc, char **argv) {
    ws_display_set_control(0);

    // Configure palettes
    ws_display_set_shade_lut_default();
    outportw(WS_SCR_PAL_PORT(0), 0x7070);
    outportw(WS_SCR_PAL_PORT(1), 0x7700);

    clearScreen();

    // Configure screen layout
    uint16_t *scr = screenData;
    for (int iy = 0; iy < WS_DISPLAY_HEIGHT_TILES; iy++, scr += 4) {
        for (int ix = 0; ix < WS_DISPLAY_WIDTH_TILES; ix++, scr++) {
            *scr = (iy + ((ix >> 1) * WS_DISPLAY_HEIGHT_TILES) + TILE_OFFSET)
                | WS_SCREEN_ATTR_PALETTE((ix & 1) ? 1 : 0);
        }
    }
    outportb(WS_SCR_BASE_PORT, SCREEN_BASE * 0x11);
    ws_display_scroll_screen1_to(256 - WS_DISPLAY_WIDTH_PIXELS, 256 - WS_DISPLAY_HEIGHT_PIXELS);
    ws_display_set_control(WS_DISPLAY_CTRL_SCR1_ENABLE);

    initGamepadUI();
    clearGraphics();
}

void clearScreen(void) {
    memFill(tileData, 0, sizeof(tileData));
}

void handleSystemEvents(void) {}

static uint16_t key_held;

enum ECommand getInput(void) {
    if (cooldown) {
        cooldown--;
        if (!cooldown)
            key_held = 0;
    }

    uint16_t key = ws_keypad_scan();
    uint16_t key_pressed = key & ~key_held;
    uint16_t key_released = key_held & ~key;
    key_held = key;

    if (!key_held)
        cooldown = 0;

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                0
        );
    }

    performAction();

    if (key_pressed & WS_KEY_X1) {
        if (waitForKey) {
            return kCommandNone;
        }

        cooldown = COOLDOWN_MAX;
        return kCommandUp;
    }

    if (key_pressed & WS_KEY_X4) {
        if (waitForKey) {
            return kCommandNone;
        }

        cooldown = COOLDOWN_MAX;
        if (key_pressed & WS_KEY_B) {
            return kCommandStrafeLeft;
        } else {
            return kCommandLeft;
        }
    }

    if (key_pressed & WS_KEY_X2) {
        if (waitForKey) {
            return kCommandNone;
        }

        cooldown = COOLDOWN_MAX;
        if (key_pressed & WS_KEY_B) {
            return kCommandStrafeRight;
        } else {
            return kCommandRight;
        }
    }

    if (key_pressed & WS_KEY_X3) {
        if (waitForKey) {
            return kCommandNone;
        }

        cooldown = COOLDOWN_MAX;
        return kCommandDown;
    }

    if (key_pressed & WS_KEY_A) {

        if (waitForKey) {
            return kCommandNone;
        }

        if (currentGameMenuState == kPlayGame) {
            playSound(3);
            cooldown = COOLDOWN_MAX;

            return performActionJoypad();
        } else {
            return kCommandFire1;
        }
    }

    if (key_pressed & WS_KEY_B) {

        if (waitForKey) {
            waitForKey = 0;
            firstFrameOnCurrentState = 1;
            needsToRedrawVisibleMeshes = 1;
            return kCommandNone;
        }

        if (currentGameMenuState == kPlayGame) {

            cursorPosition = (cursorPosition + 1);
            playSound(2);
            if (cursorPosition >= 6) {
                cursorPosition = 0;
            }

            HUD_initialPaint();
            cooldown = COOLDOWN_MAX;
            return kCommandNone;
        } else {
            return kCommandFire1;
        }
    }

    return kCommandNone;
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {
    size_t len = strlen(text);
    uint16_t dstX = x * 8;
    int8_t dstY = y * 8;

    uint8_t c;
    uint8_t d;
    uint8_t lastSpacePos = 0xFF;

    for (c = 0; c < len; ++c) {

        char currentChar = text[c];

        if (currentChar == '-') {
            currentChar = charToReplaceHifenWith;
        }

        if (currentChar == '\n' || dstX >= (margin)) {
            dstX = x * 8;
            dstY += 8;
            continue;
        }

        if (dstY >= YRES_FRAMEBUFFER) {
            return;
        }

        if (currentChar == ' ') {
            lastSpacePos = c;
        } else {
            if ((c - 1) == lastSpacePos) {
                d = c;
                while (d < len && text[d] != ' ') ++d;

                if ((dstX + ((d - c ) * 8)) >= margin ) {
                    dstX = x * 8;
                    dstY += 8;
                }
            }
        }


        if (currentChar >= 'a') {
            if (currentChar <= 'z') {
                currentChar = (currentChar - 'a') + 'A';
            } else {
                currentChar -= ('z' - 'a');
            }
        }

        const uint8_t __far* fontTop = &font[((currentChar - 32) << 3)];

        for (uint8_t f = 0; f < 8; ++f) {
            uint8_t e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                realPut(dstX + (7 - e), dstY + (f), (chunk & 1), NULL);
                chunk = chunk >> 1;
            }

            fontTop++;
        }

        dstX += 8;
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

    uint16_t tile_src = 0;
    uint16_t tile_dst = 0;
    for (int ix = 0; ix < 8; ix++, tile_src += 16, tile_dst += WS_DISPLAY_HEIGHT_TILES) {
        memcpy(
            tileData + (tile_dst * WS_DISPLAY_TILE_SIZE),
            imageBuffer + (tile_src * WS_DISPLAY_TILE_SIZE),
            16 * WS_DISPLAY_TILE_SIZE
        );
    }

    clearGraphics();
}


void enterTextMode(void) {}

void exitTextMode(void) {
    clearScreen();
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;

    for (x = x0; x < x1; ++x) {
        uint8_t *ptr = &tileData[(y0 << 1) + ((x & 8) >> 3) + ((x & 240) * WS_DISPLAY_HEIGHT_TILES)];
        uint8_t v = 128 >> (x & 7);

        if (colour) {
            for (y = y0; y < y1; ++y, ptr += 2) {
                if (!stipple || ((x + y) & 1))
                   *ptr |= v;
            }
        } else {
            for (y = y0; y < y1; ++y, ptr += 2) {
                if (!stipple || ((x + y) & 1))
                   *ptr &= ~v;
            }
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
        if (sx > 0 ? (x0 >= x1) : (x0 <= x1))
            if (sy > 0 ? (y0 >= y1) : (y0 <= y1))
                break;

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
