#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"

#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

SDL_Window *window;
SDL_Renderer *renderer;

uint8_t getPaletteEntry(const uint32_t origin) {
    uint8_t shade;

    if (!(origin & 0xFF000000)) {
        return TRANSPARENCY_COLOR;
    }

    shade = 0;
    shade += (((((origin & 0x0000FF)) << 2) >> 8)) << 6;
    shade += (((((origin & 0x00FF00) >> 8) << 3) >> 8)) << 3;
    shade += (((((origin & 0xFF0000) >> 16) << 3) >> 8)) << 0;

    return shade;
}


void graphicsPut(int x, int y, uint8_t pixel ) {
    x = x * 2;
    framebuffer[(320 * y) + x] = pixel;
    framebuffer[(320 * y) + x + 1] = pixel;
}

void fix_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t pixel) {

    if (x0 == x1) {

        int16_t _y0 = y0;
        int16_t _y1 = y1;

        if (y0 > y1) {
            _y0 = y1;
            _y1 = y0;
        }


        for (int16_t y = _y0; y <= _y1; ++y) {
            if (x0 < 0 || x0 >= 256 || y < 0 || y >= 128) {
                continue;
            }

            graphicsPut(x0, y, pixel );
        }
        return;
    }

    if (y0 == y1) {
        int16_t _x0 = x0;
        int16_t _x1 = x1;

        if (x0 > x1) {
            _x0 = x1;
            _x1 = x0;
        }

        for (int16_t x = _x0; x <= _x1; ++x) {
            if (x < 0 || x >= 256 || y0 < 0 || y0 >= 128) {
                continue;
            }

            graphicsPut(x, y0, pixel );
        }
        return;
    }

    //switching x0 with x1
    if (x0 > x1) {
        x0 = x0 + x1;
        x1 = x0 - x1;
        x0 = x0 - x1;

        y0 = y0 + y1;
        y1 = y0 - y1;
        y0 = y0 - y1;
    }

    {
        //https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

        int dx = abs(x1 - x0);
        int sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0);
        int sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;  /* error value e_xy */

        while (1) {
            framebuffer[(320 * y0) + (2 * x0)] = pixel;
            /* loop */
            if (x0 == x1 && y0 == y1) return;
            int e2 = 2 * err;

            if (e2 >= dy) {
                err += dy; /* e_xy+e_x > 0 */
                x0 += sx;
            }

            if (e2 <= dx) {
                /* e_xy+e_y < 0 */
                err += dx;
                y0 += sy;
            }
        }
    }
}

void graphicsHorizontalLine(int16_t x0, int16_t x1, int16_t y, uint8_t pixel) {
    fix_line(x0, y, x1, y, pixel);
}

void graphicsVerticalLine(int16_t x0, int16_t y0, int16_t y1, uint8_t pixel ) {
    fix_line(x0, y0, x0, y1, pixel);
}

#ifdef __EMSCRIPTEN__
void enterFullScreenMode() {
    EmscriptenFullscreenStrategy s;
    memset(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}
#endif


void graphicsInit() {
    int r, g, b;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

    window =
            SDL_CreateWindow("The Mistral Report", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);

    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palette[paletteEntry] = pixel;
            }
        }
    }

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
    defaultFont = loadBitmap("font.img");
}

void handleSystemEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }

        if (event.type == SDL_KEYDOWN) {

            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = kCommandFire1;
                    break;

                case SDLK_ESCAPE:
                case SDLK_q:
                    mBufferedCommand = kCommandBack;
                    break;

                case SDLK_SPACE:

                case SDLK_s:
                    mBufferedCommand = kCommandStrafeLeft;
                    break;
                case SDLK_d:
                    mBufferedCommand = kCommandStrafeRight;
                    break;

                case SDLK_x:
                    mBufferedCommand = kCommandFire2;
                    break;
                case SDLK_c:
                    mBufferedCommand = kCommandFire3;
                    break;
                case SDLK_e:
                    mBufferedCommand = kCommandFire4;
                    break;

                case SDLK_LEFT:
                    mBufferedCommand = kCommandLeft;
                    break;
                case SDLK_RIGHT:
                    mBufferedCommand = kCommandRight;
                    break;
                case SDLK_UP:
                    mBufferedCommand = kCommandUp;
                    break;
                case SDLK_DOWN:
                    mBufferedCommand = kCommandDown;
                    break;

                default:
                    return;
            }
        }
    }
}

void graphicsShutdown() {
    SDL_Quit();

    releaseBitmap(defaultFont);
}

unsigned long t0, t1, t2;

void startup() {}

unsigned long getMilliseconds() {
    return 3;
}

void flipRenderer() {
    SDL_Rect rect;
    uint32_t pixel;
    int x, y;

    for (y = 0; y < 200; ++y) {

        if ( y < dirtyLineY0 || y > dirtyLineY1 ) {
            continue;
        }

        for (x = 0; x < 320; ++x) {

            rect.x = 2 * x;
            rect.y = (24 * y) / 10;
            rect.w = 2;
            rect.h = 3;

            pixel = palette[framebuffer[(320 * y) + x]];

            SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
                                   ((pixel & 0x0000FF00) >> 8) - 0x18,
                                   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif
}

void clearRenderer() {}
