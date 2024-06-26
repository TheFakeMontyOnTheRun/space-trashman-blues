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
#include "Common.h"

extern uint8_t firstFrameOnCurrentState;
extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
SDL_Window *window;
SDL_Renderer *renderer;

uint32_t palette[16];
uint8_t framebuffer[128 * 128];
uint8_t vfb[256 * 192];

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
    memFill(framebuffer, 0, 128 * 128);
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

        if (x0 >= 0 && y0 >= 0 && x0 < 256 && y0 < 192) {
            realPut(x0, y0, colour, NULL);
        } else {
            return;
        }

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
            put_sprite_8(
                    (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                    (cameraZ * 3) + 10,
                    &playerPositionSprite[playerDirection][0],
                    0
            );

            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_1:
                    return kCommandFire1;

                case SDLK_ESCAPE:
                    return kCommandBack;

                case SDLK_q:
                    return kCommandQuit;

                case SDLK_KP_7:
                case SDLK_2:
                    if (waitForKey) {
                        waitForKey = 0;
                        firstFrameOnCurrentState = 1;
                        needsToRedrawVisibleMeshes = 1;
                        return kCommandNone;
                    }
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
                case SDLK_6:
                    return kCommandFire6;

                case SDLK_LEFT:
                    return kCommandLeft;

                case SDLK_RIGHT:
                    return kCommandRight;

                case SDLK_z:
                    return kCommandStrafeLeft;

                case SDLK_x:
                    return kCommandStrafeRight;

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
                    realPut(dstX + (7 - e), dstY + (f), 1, NULL);
                } else {
                    realPut(dstX + (7 - e), dstY + (f), 0, NULL);
                }
                chunk = chunk >> 1;
            }

            fontTop++;
        }
        dstX += 8;
    }
}

void initHW(int argc, char **pString) {
    initKeyboardUI();

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
    if (needsToRedrawVisibleMeshes) {
        flipRenderer();
        put_sprite_8(
                (XRES_FRAMEBUFFER / 2) + ((cameraX + 6) * 3) - 1,
                (cameraZ * 3) + 10,
                &playerPositionSprite[playerDirection][0],
                1
        );

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
