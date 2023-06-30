#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "Common.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Globals.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Core.h"
#include "Engine.h"
#include "Dungeon.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

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

SDL_Window *window;
SDL_GLContext glContext;
int snapshotSignal = '.';

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

int turning = 0;
int leanX = 0;
int leanY = 0;

void graphicsInit() {
    int r, g, b;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

    window =
            SDL_CreateWindow("Sub Mare Imperium - Derelict", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    glContext = SDL_GL_CreateContext(window);

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
    defaultFont = loadBitmap("font.img");
    enableSmoothMovement = TRUE;

    initGL();
}

void handleSystemEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }

        if (event.type == SDL_KEYUP) {
            visibilityCached = FALSE;
            needsToRedrawVisibleMeshes = TRUE;
        }

        if (event.type == SDL_KEYDOWN) {
            leanY = leanX = 0;
            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = kCommandFire1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
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
                case SDLK_v:
                    mBufferedCommand = kCommandFire4;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_t:
                    leanY = -1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_g:
                    leanY = 1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_f:
                    leanX = -1;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_h:
                    leanX = 1;
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

                case SDLK_i:
                    visibilityCached = FALSE;
                    break;
                case SDLK_o:
                    visibilityCached = FALSE;
                    break;

                case SDLK_j:
                    visibilityCached = FALSE;
                    break;
                case SDLK_k:
                    visibilityCached = FALSE;
                    break;


                case SDLK_LEFT:
                    turning = 1;
                    leanX = -ANGLE_TURN_STEP;
                    break;
                case SDLK_RIGHT:
                    leanX = ANGLE_TURN_STEP;
                    turning = 1;
                    break;
                case SDLK_UP:
                    mBufferedCommand = kCommandUp;
                    visibilityCached = FALSE;
                    break;
                case SDLK_1:
                    enableSmoothMovement = TRUE;
                    break;

                case SDLK_2:
                    enableSmoothMovement = FALSE;
                    break;

                case SDLK_3:
                    leanX = -ANGLE_TURN_STEP;
                    break;

                case SDLK_4:
                    leanX = ANGLE_TURN_STEP;

                    break;

                case SDLK_DOWN:
                    mBufferedCommand = kCommandDown;
                    visibilityCached = FALSE;
                    break;

                case SDLK_n:
                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;
                    break;
                case SDLK_m:
                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;
                    break;

                default:
                    return;
            }
        }
    }
}

void graphicsShutdown() {
    SDL_DestroyWindow(window);
    SDL_Quit();

    releaseBitmap(defaultFont);

    texturesUsed = 0;
}

void flipRenderer() {
    SDL_GL_SwapWindow(window);

#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif
}
