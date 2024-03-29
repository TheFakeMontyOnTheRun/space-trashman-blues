#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <Core.h>
#include <assert.h>

#include "Enums.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"
#include "KeyboardUI.h"
#include "UI.h"
#include "font.h"

#include "SDL.h"

extern uint8_t firstFrameOnCurrentState;
extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
SDL_Window *window;
SDL_Renderer *renderer;
uint8_t updateDirection;

uint32_t palette[16];
uint8_t framebuffer[128 * 128];
uint8_t vfb[256 * 192];

void graphicsPut(uint8_t x, uint8_t y) {
    framebuffer[(128 * y) + x] = 1;
#ifdef PUTAFLIP
    graphicsFlush();
    SDL_Delay(100);
#endif
}

void graphicsPutPointArray(uint8_t *y128Values) {
    uint8_t *stencilPtr = y128Values;
    int x;

    for (x = 0; x < XRES; ++x) {
        graphicsPut(x, *stencilPtr);
        ++stencilPtr;
    }
}

void clearTextScreen(void) {
    fillRect(0, 129, 256, 192, 0, 0);
}

void enterTextMode(void) {
}

void exitTextMode(void) {
}

void vLine(uint8_t x0, uint8_t y0, uint8_t y1, uint8_t shouldStipple) {
    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }


    for (y = _y0; y <= _y1; ++y) {
        if (!shouldStipple || (y & 1)) {
            graphicsPut(x0, y);
        }
    }
}

void shutdownGraphics(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void clearGraphics(void) {
    memset(framebuffer, 0, 128 * 128);
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

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {
    vfb[(256 * y) + x] = colour;

    return NULL;
}

void clearScreen(void) {
    fillRect(0, 0, 256, 192, 0, 0);
}

void handleSystemEvents(void) {
    clearGraphics();
}

enum ECommand getInput(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            return kCommandQuit;
        }

        if (event.type == SDL_KEYUP) {

            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_1:
                    if (waitForKey) {
                        waitForKey = 0;
                        firstFrameOnCurrentState = 1;
                        needs3dRefresh = 1;
                        return kCommandNone;
                    }
                    return kCommandFire1;

                case SDLK_ESCAPE:
                    return kCommandBack;

                case SDLK_q:
                    return kCommandQuit;

                case SDLK_KP_7:
                case SDLK_2:
                    return kCommandFire2;

                case SDLK_KP_8:
                case SDLK_3:
                    return kCommandFire3;

                case SDLK_KP_4:
                case SDLK_4:
                    return kCommandFire4;

                case SDLK_KP_5:
                case SDLK_5:
                    return kCommandFire5;

                case SDLK_KP_9:
                case SDLK_9:
                    return kCommandFire6;

                case SDLK_s:
                    clearTextScreen();
                    break;

                case SDLK_LEFT:
                    updateDirection = 1;
                    return kCommandLeft;

                case SDLK_RIGHT:
                    updateDirection = 1;
                    return kCommandRight;

                case SDLK_UP:
                    return kCommandUp;

                case SDLK_DOWN:
                    return kCommandDown;

                default:
                    return kCommandNone;
            }
        }
    }

    performAction();

    return kCommandNone;
}

void writeStrWithLimit(uint8_t _x, uint8_t y, const char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {
    uint8_t len = strlen(text);
    char *ptr = text;
    uint8_t c = 0;
    uint8_t x = _x;

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

        uint8_t *fontTop = &font[((cha - 32) << 3)];


        for (int d = 0; d < 8; ++d) {
            int e;
            uint8_t chunk = *fontTop;

            for (e = 0; e < 8; ++e) {
                if (chunk & 1) {
                    realPut(8 * x + (7 - e), 8 * y + (d), 1, NULL);
                } else {
                    realPut(8 * x + (7 - e), 8 * y + (d), 0, NULL);
                }
                chunk = chunk >> 1;
            }


            fontTop++;
        }

        ++x;
        ++ptr;
    }
}

void initHW(int, char **pString) {
    initKeyboardUI();
    updateDirection = 1;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    clearGraphics();
    window =
            SDL_CreateWindow("Derelict 8-bits SDL2 test", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 512, 384, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);

    palette[0] = 0xFF000099;
    palette[1] = 0xFFFFFFBF;
    palette[2] = 0xFFE0FFFF;
    palette[3] = 0xFFFF0000;
    palette[4] = 0xFFFFFFFF;
    palette[5] = 0xFF000000;
    palette[6] = 0xFF0000FF;
    palette[7] = 0xFFFF00FF;
    palette[8] = 0xFF00b7eb;
    palette[9] = 0xFFFFFF00;
    palette[10] = 0xFFAFEEEE;
    palette[11] = 0xFFffc0cb;
    palette[12] = 0xFF00FF00;
    palette[13] = 0xFFAAFFAA;
    palette[14] = 0xFF0000FF;
    palette[15] = 0xFFAAAAFF;

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
}


void flipRenderer(void) {
    int x, y;
    for (y = 0; y < 128; ++y) {
        for (x = 0; x < 128; ++x) {
            int index = framebuffer[(128 * y) + x];

            if (index < 0 || index >= 16) {
                continue;
            }

            realPut(x, y, index, NULL);
        }
    }
}


void flushVirtualFramebuffer(void) {
    int x, y, r, g, b;
    uint32_t pixel;
    SDL_Rect rect;

    for (y = 0; y < 192; ++y) {
        for (x = 0; x < 256; ++x) {
            int index = vfb[(256 * y) + x];
            rect.x = 2 * x;
            rect.y = 2 * y;
            rect.w = 2;
            rect.h = 2;


            if (index < 0 || index >= 16) {
                continue;
            }

            pixel = palette[index];

            r = (pixel & 0x00FF0000) >> 16;
            g = ((pixel & 0x0000FF00) >> 8);
            b = ((pixel & 0x000000FF));

            SDL_SetRenderDrawColor(renderer,
                                   r,
                                   g,
                                   b,
                                   255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    SDL_RenderPresent(renderer);
}

void startFrame(int x, int y, int width, int height) {

}

void endFrame(void) {
    if (needs3dRefresh) {
        if (updateDirection) {
            updateDirection = 0;
            switch (getPlayerDirection()) {
                case 0:
                    writeStrWithLimit(12, 17, "N", 31, 2, 0);
                    break;
                case 1:
                    writeStrWithLimit(12, 17, "E", 31, 2, 0);
                    break;
                case 2:
                    writeStrWithLimit(12, 17, "S", 31, 2, 0);
                    break;
                case 3:
                    writeStrWithLimit(12, 17, "W", 31, 2, 0);
                    break;
            }
        }

        flipRenderer();
    }
    flushVirtualFramebuffer();
}


void fillRect(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour, uint8_t stipple) {
    int x, y;
    for (y = y0; y < y1; ++y) {
        for (x = x0; x < x1; ++x) {
            if (!stipple || ((x + y) & 1 )) {
                realPut(x, y, colour, NULL);
            }
        }
    }
}

uint8_t getPaletteEntry(uint32_t colour) {
    return colour & 3;
}
