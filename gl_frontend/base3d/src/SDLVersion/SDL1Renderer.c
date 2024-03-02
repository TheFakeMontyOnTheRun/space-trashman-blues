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
#include "Mesh.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"

#ifdef __APPLE__

#include <SDL.h>

#else

#include <SDL.h>
#include <SDL_mixer.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif
#endif

#define ANGLE_TURN_THRESHOLD 40
#define ANGLE_TURN_STEP 5

int turning = 0;
int leanX = 0;
int leanY = 0;

SDL_Surface *video;

void graphicsInit(void) {
    SDL_Init(SDL_INIT_EVERYTHING);
    video = SDL_SetVideoMode(320, 240, 32, SDL_OPENGL);

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
    defaultFont = loadBitmap("font.img");
    enableSmoothMovement = TRUE;
    initGL();
}

void handleSystemEvents(void) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {
            mBufferedCommand = kCommandQuit;
            return;
        }

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
    SDL_GL_SwapBuffers();

#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif
}
