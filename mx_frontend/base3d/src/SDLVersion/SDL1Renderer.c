#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Core.h"
#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "Renderer.h"

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

void enterFullScreenMode(void) {
    EmscriptenFullscreenStrategy s;
    memFill(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}
#endif

SDL_Surface *video;
SDL_Surface *stretchedBuffer;

uint8_t getPaletteEntry(uint32_t origin) {
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

void graphicsInit(void) {
    int r, g, b, c;

    SDL_Init(SDL_INIT_EVERYTHING);
    video = SDL_SetVideoMode(320, 240, 32, 0);
    for (r = 0; r < 256; r += 16) {
        for (g = 0; g < 256; g += 8) {
            for (b = 0; b < 256; b += 8) {
                uint32_t pixel = 0xFF000000 + (r << 16) + (g << 8) + (b);
                uint8_t paletteEntry = getPaletteEntry(pixel);
                palette[paletteEntry] = SDL_MapRGB(video->format, (((pixel & 0x000000FF))) - 0x38,
                                                   (((pixel & 0x0000FF00) >> 8)) - 0x18,
                                                   (((pixel & 0x00FF0000) >> 16)) - 0x10);
            }
        }
    }

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
    defaultFont = loadBitmap("font.img");
    enableSmoothMovement = TRUE;

    stretchedBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, 32, video->format->Rmask, video->format->Gmask,
                                           video->format->Bmask, video->format->Amask);
}

void handleSystemEvents(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
#ifdef EMIT_QUIT_OPTION
        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }
#endif
        if (event.type == SDL_KEYDOWN) {

            SDLKey key = event.key.keysym.sym;


            switch (event.key.keysym.scancode) {
                case 461:
                    mBufferedCommand = kCommandBack;
                    visibilityCached = FALSE;
                    break;

                case 403:
                    mBufferedCommand = kCommandFire1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case 404:
                    mBufferedCommand = kCommandFire2;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case 405:
                    mBufferedCommand = kCommandFire3;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case 406:
                    mBufferedCommand = kCommandBack;
                    visibilityCached = FALSE;
                    break;

                case 412:
                    mBufferedCommand = kCommandStrafeLeft;
                    visibilityCached = FALSE;
                    break;
                case 417:
                    mBufferedCommand = kCommandStrafeRight;
                    visibilityCached = FALSE;
                    break;
            }

            switch (key) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = kCommandFire1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_ESCAPE:
                case SDLK_q:
                    mBufferedCommand = kCommandBack;
                    visibilityCached = FALSE;
                    break;

                case SDLK_SPACE:

                case SDLK_s:
                    mBufferedCommand = kCommandStrafeLeft;
                    visibilityCached = FALSE;
                    break;
                case SDLK_d:
                    mBufferedCommand = kCommandStrafeRight;
                    visibilityCached = FALSE;
                    break;

                case SDLK_v:
                    visibilityCached = FALSE;
                    break;
                case SDLK_b:
                    visibilityCached = FALSE;
                    break;

                case SDLK_j:
                    visibilityCached = FALSE;
                    break;
                case SDLK_k:
                    visibilityCached = FALSE;
                    break;

                case SDLK_x:
                    mBufferedCommand = kCommandFire2;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_c:
                    mBufferedCommand = kCommandFire3;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;
                case SDLK_e:
                    mBufferedCommand = kCommandFire4;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_LEFT:
                    mBufferedCommand = kCommandLeft;
                    visibilityCached = FALSE;
                    break;
                case SDLK_RIGHT:
                    mBufferedCommand = kCommandRight;
                    visibilityCached = FALSE;
                    break;
                case SDLK_UP:
                    mBufferedCommand = kCommandUp;
                    visibilityCached = FALSE;
                    break;
                case SDLK_DOWN:
                    mBufferedCommand = kCommandDown;
                    visibilityCached = FALSE;
                    break;

                default:
                    return;
            }
        }
    }
}

void graphicsShutdown(void) {
    SDL_Quit();

    releaseBitmap(defaultFont);

    texturesUsed = 0;
}

void flipRenderer(void) {

    int x = 0, y = 0;
    int dstY = 0;
    int scaller = 0;
    int even = 0;
    int heightY = 1;

    uint8_t *src;
    uint32_t *dst;

    uint8_t newFrame[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

    renderPageFlip(newFrame, framebuffer,
                   previousFrame, turnStep, turnTarget, 0);

    SDL_LockSurface(stretchedBuffer);


    for (y = 0; y < YRES_FRAMEBUFFER; ++y) {
        int chunky;

        if (scaller == 4) {
            heightY = 2;
        } else {
            heightY = 1;
        }


        for (chunky = 0; chunky < heightY; ++chunky) {
            dst = (uint32_t *) stretchedBuffer->pixels;
            src = &newFrame[(XRES_FRAMEBUFFER * y)];
            dst += (XRES_FRAMEBUFFER * (dstY + chunky));

            for (x = 0; x < XRES_FRAMEBUFFER; ++x) {
                *dst++ = palette[*src++];
            }
        }

        dstY++;
        scaller++;


        if (scaller == 5) {
            scaller = 0;
            dstY++;
        }
    }

    SDL_UnlockSurface(stretchedBuffer);
    SDL_FillRect(video, NULL, SDL_MapRGB(video->format, 0, 0, 0));
    SDL_BlitSurface(stretchedBuffer, NULL, video, NULL);

    SDL_Flip(video);
#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif


}

void clearRenderer(void) {
    SDL_FillRect(video, NULL, 0);
}
