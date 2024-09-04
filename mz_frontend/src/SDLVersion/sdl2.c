#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <Core.h>
#include <assert.h>

#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "Common.h"
#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "UI.h"
#include "KeyboardUI.h"

#include "SDL.h"
#include "font.h"

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
SDL_Window *window;
SDL_Renderer *renderer;

#define TRANSPARENCY_COLOR 17

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[256 * 160];

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

void graphicsPut(int16_t x, int16_t y, uint16_t colour) {
    if (x < 0) {
        x = 0;
    }

    if (x >= 128) {
        x = 127;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= 128) {
        y = 127;
    }

    assert( !(colour < 0 || colour >= 16));
    framebuffer[(256 * y) + x] = colour;
#ifdef PUTAFLIP
    graphicsFlush();
    SDL_Delay(100);
#endif
}

void hLine(int16_t x0, int16_t x1, int16_t y, uint16_t colour) {
    if (y < 0 || y > YRESMINUSONE) {
        return;
    }

    int16_t _x0 = x0;
    int16_t _x1 = x1;

    if (x0 > x1) {
        _x0 = x1;
        _x1 = x0;
    }

    if (_x0 < 0) {
        _x0 = 0;
    }

    if (_x1 > XRESMINUSONE) {
        _x1 = XRESMINUSONE;
    }

    for (int x = _x0; x <= _x1; ++x) {
        framebuffer[(XRES_FRAMEBUFFER * y) + x] = colour;
    }
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint16_t colour) {

    if (x0 < 0 || x0 > XRESMINUSONE) {
        return;
    }

    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

    if (_y0 < 0) {
        _y0 = 0;
    }

    if (_y1 >= 128) {
        _y1 = 127;
    }

    for (y = _y0; y <= _y1; ++y) {
        framebuffer[(256 * y) + x0] = colour;
    }
}

void shutdownGraphics(void) {
    SDL_Quit();
}

void realPut(int x, int y, uint8_t value) {
    if (x < 0) {
        x = 0;
    }

    if (x >= 256) {
        x = 255;
    }

    if (y < 0) {
        y = 0;
    }

    if (y >= 160) {
        y = 159;
    }
    assert( !(value < 0 || value >= 16));
    framebuffer[(XRES_FRAMEBUFFER * y) + x] = value;
}

void clearGraphics(void) {
    memFill(framebuffer, 0, 256 * 160);
}

void clearScreen(void) {}

void put_sprite_8(uint16_t x, uint8_t y, uint8_t *sprite, uint8_t colour) {

    for(uint8_t c = 0; c < 8; ++c) {
        uint8_t line = *sprite;
        for (uint16_t d = 0; d < 8; ++d) {
            if (line & 1) {
                realPut( x + d, y + c, colour);
            }
            line = line >> 1;
        }
        ++sprite;
    }
}

void handleSystemEvents(void) {
    SDL_Event event;

    mBufferedCommand = '.';

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {

            mBufferedCommand = 'q';
        }

        if (event.type == SDL_KEYUP) {

            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = 'a';
                    break;

                case SDLK_ESCAPE:
                case SDLK_q:
                    SDL_Quit();
                    exit(0);
                    break;

                case SDLK_SPACE:
                    break;

                case SDLK_KP_7:
                case SDLK_7:
                    mBufferedCommand = '7';
                    break;

                case SDLK_KP_8:
                case SDLK_8:
                    mBufferedCommand = '8';
                    break;


                case SDLK_KP_9:
                case SDLK_9:
                    mBufferedCommand = '9';
                    break;


                case SDLK_s:
                    break;
                case SDLK_d:
                    break;
                case SDLK_v:
                    break;
                case SDLK_b:
                    break;
                case SDLK_j:
                    break;
                case SDLK_k:
                    break;
                case SDLK_x:
                    mBufferedCommand = 'd';
                    break;
                case SDLK_c:
                    break;
                case SDLK_e:
                    break;

                case SDLK_1:
                    mBufferedCommand = '1';
                    break;

                case SDLK_2:
                    mBufferedCommand = '2';
                    break;

                case SDLK_3:
                    mBufferedCommand = '3';
                    break;

                case SDLK_4:
                    mBufferedCommand = '4';
                    break;

                case SDLK_5:
                    mBufferedCommand = '5';
                    break;

                case SDLK_6:
                    mBufferedCommand = '6';
                    break;


                case SDLK_LEFT:
                    mBufferedCommand = 'q';
                    break;
                case SDLK_RIGHT:
                    mBufferedCommand = 'e';
                    break;
                case SDLK_UP:
                    mBufferedCommand = 'w';
                    break;
                case SDLK_DOWN:
                    mBufferedCommand = 's';
                    break;
                default:
                    mBufferedCommand = '.';
                }
            }
        }
}

enum ECommand getInput(void) {

    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                0
        );
    }

    switch(mBufferedCommand) {
        case 'q':
            return kCommandLeft;
        case 'w':
            return kCommandUp;
        case 's':
            return kCommandDown;
        case 'e':
            return kCommandRight;
        case 'a':
            return kCommandStrafeLeft;
        case 'd':
            return kCommandStrafeRight;
        case 'l':
            return kCommandBack;

        case '1':
            if (waitForKey) {
                waitForKey = 0;
                firstFrameOnCurrentState = 1;
                needsToRedrawVisibleMeshes = 1;
                return kCommandNone;
            }

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

void initHW(int argc, char **argv) {
    renderer = NULL;
    int r, g, b;
    mBufferedCommand = '.';
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    memFill(framebuffer, 0, 256 * 160);
    window =
            SDL_CreateWindow("Derelict 16-bits SDL2 test", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 512, 320, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);

    palette[0] = 0x000000;
    palette[1] = 0x0000AA;
    palette[2] = 0x00AA00;
    palette[3] = 0x00AAAA;
    palette[4] = 0xAA0000;
    palette[5] = 0xAA00AA;
    palette[6] = 0xAA5500;
    palette[7] = 0xAAAAAA;
    palette[8] = 0x555555;
    palette[9] = 0x5555FF;
    palette[10] = 0x55FF55;
    palette[11] = 0x55FFFF;
    palette[12] = 0xFF5555;
    palette[13] = 0xFF55FF;
    palette[14] = 0xFFFF55;
    palette[15] = 0xFFFFFF;

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif

    initKeyboardUI();
    clearGraphics();
}


void flipRenderer(void) {
    SDL_Rect rect;
    uint32_t pixel;
    int x, y;

    rect.x = 0;
    rect.y = 0;
    rect.w = 512;
    rect.h = 320;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    for (y = 0; y < 160; ++y) {
        for (x = 0; x < 255; ++x) {
            rect.x = 2 * x;
            rect.y = 2 * y;
            rect.w = 2;
            rect.h = 2;
            int index = framebuffer[(256 * y) + x];
            assert( !(index < 0 || index >= 16));

            pixel = palette[index];

            int r = (pixel & 0x00FF0000) >> 16;
            int g = ((pixel & 0x0000FF00) >> 8);
            int b = ((pixel & 0x000000FF));

            SDL_SetRenderDrawColor(renderer, r,
                                   g,
                                   b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif
}

void drawTextAtWithMarginWithFiltering(const int x, const int y, int margin, const char *text, const uint8_t fg,
                                       char charToReplaceHifenWith) {

    size_t len = strlen(text);
    int32_t dstX = x * 8;
    int32_t dstY = y * 8;

    size_t c;
    size_t d;
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

        uint8_t *fontTop = &font[((currentChar - 32) << 3)];

        for (int f = 0; f < 8; ++f) {
            int e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    realPut(dstX + (7 - e), dstY + (f), 1);
                } else {
                    realPut(dstX + (7 - e), dstY + (f), 0);
                }
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
    if (currentGameMenuState == kPlayGame) {
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                1
        );
    }
    flipRenderer();
}


void clearTextScreen(void) {
    int c, d;
    for (c = 16; c < 24; ++c) {
        for (d = 0; d < 32; ++d) {
            drawTextAtWithMarginWithFiltering(d, c, 256, " ", 2, ' ');
        }
    }
}

void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            realPut(x, y, colour);
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

        realPut(x0, y0, colour);

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

