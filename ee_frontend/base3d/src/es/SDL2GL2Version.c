#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
#include "Renderer.h"

#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>

void enterFullScreenMode(void) {
    EmscriptenFullscreenStrategy s;
    memset(&s, 0, sizeof(s));
    s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
    s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    emscripten_enter_soft_fullscreen(0, &s);
}
#endif


#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

extern int turning;
extern int leanX;
extern int leanY;

int snapshotSignal = '.';
int enable3DRendering = FALSE;

const int width = 640;
const int height = 480;

SDL_Window *window;
SDL_GLContext context;

time_t lastMouseButtonTime = 0;
time_t lastClickTime = 0;
uint8_t sucessiveClicks = 0;

void graphicsInit(void) {

    enableSmoothMovement = TRUE;
    defaultFont = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    window = SDL_CreateWindow(
            "", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
    }

    context = SDL_GL_CreateContext(window);

    if (context == NULL) {
        printf(
                "OpenGL context could not be created! SDL Error: %s\n",
                SDL_GetError());
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    initGL();

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
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

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            lastMouseButtonTime = SDL_GetTicks();
            time_t timeDiff = (SDL_GetTicks() - lastClickTime);

            if (timeDiff > 500 ) {
                lastClickTime = 0;
                sucessiveClicks = 0;
            }

            return;
        }

        if (event.type == SDL_MOUSEBUTTONUP) {
            time_t timeDiff = (SDL_GetTicks() - lastMouseButtonTime);
            if (timeDiff >= 500) {
                sucessiveClicks = 0;
                /* long press */
                mBufferedCommand = kCommandFire2;
                needsToRedrawVisibleMeshes = TRUE;
                visibilityCached = FALSE;

            } else {
                sucessiveClicks++;

                if (sucessiveClicks >= 2) {
                    mBufferedCommand = kCommandFire1;
                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;

                } else if (sucessiveClicks == 1) {

                    if ( ((height / 3) > event.button.y)) {
                        mBufferedCommand = kCommandUp;
                    }

                    if ( (event.button.y > (2* height / 3)) ) {
                        mBufferedCommand = kCommandDown;
                    }

                    if ( ((height / 3) < event.button.y) && (event.button.y < (2* height / 3)) ) {
                        if ( event.button.x  < (width / 2) ) {
                            turning = 1;
                            leanX = -ANGLE_TURN_STEP;
                        } else {
                            turning = 1;
                            leanX = ANGLE_TURN_STEP;
                        }
                    }

                    needsToRedrawVisibleMeshes = TRUE;
                    visibilityCached = FALSE;
                }
            }
            lastClickTime = SDL_GetTicks();
            lastMouseButtonTime = 0;
            return;
        }

        if (event.type == SDL_KEYUP) {
            needsToRedrawVisibleMeshes = TRUE;
        }

        if (event.type == SDL_KEYDOWN) {
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
                case SDLK_k:
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                    break;
                case SDLK_w:
                    SDL_SetWindowFullscreen(window, 0);
                    break;

                case SDLK_t:
                    leanY = -45;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_g:
                    leanY = 45;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_f:
                    leanX = -45;
                    visibilityCached = FALSE;
                    needsToRedrawVisibleMeshes = TRUE;
                    break;

                case SDLK_h:
                    leanX = 45;
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

                case SDLK_LEFT:
                    turning = 1;
                    leanX = -ANGLE_TURN_STEP;
                    break;
                case SDLK_RIGHT:
                    turning = 1;
                    leanX = ANGLE_TURN_STEP;
                    break;
                case SDLK_UP:
                    mBufferedCommand = kCommandUp;
                    break;

                case SDLK_u:
                    unloadTextures();
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
                case SDLK_l:
                    graphicsShutdown();
                    exit(0);
                    return;

                default:
                    return;
            }
        }
    }
}

void graphicsShutdown(void) {
    texturesUsed = 0;

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void flipRenderer(void) {
    SDL_GL_SwapWindow(window);
}
