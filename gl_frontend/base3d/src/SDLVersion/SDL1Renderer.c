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
#include "CRenderer.h"

#ifdef __APPLE__

#include <SDL.h>

#else

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif
#endif

SDL_Surface *video;


//SDL_AudioDeviceID deviceId;
//
//char* audioFiles[] = {"./data/wav/menu_move.wav", "./data/wav/menu_select.wav", "./data/wav/gotclue.wav", "./data/wav/detected2.wav", "./data/wav/bong.wav", "./data/wav/fire.wav", "./data/wav/enemyfire.wav", "./data/wav/detected2.wav" };
//Mix_Chunk *audioBuffers[8];
//int audioEnabled = 0;
//
//void setupOPL2(int port) {
//
//}
//
//void stopSounds() {
//    audioEnabled = 0;
//}
//
//
//void playSound( const int action ){
//    if (audioEnabled ) {
//        Mix_PlayChannel( -1, audioBuffers[action], 0 );
//    }
//}
//
//void soundTick() {}
//
//void muteSound() {}



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


  SDL_Init(SDL_INIT_EVERYTHING);
  video = SDL_SetVideoMode(640, 480, 32, SDL_OPENGL);

#ifdef __EMSCRIPTEN__
  enterFullScreenMode ();
#endif
  defaultFont = loadBitmap("font.img");
  enableSmoothMovement = TRUE;

//    if ( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 ) {
//        audioEnabled = 0;
//    } else {
//
//        for (c = 0; c < 8; ++c) {
//            audioBuffers[c] = Mix_LoadWAV( audioFiles[c] );
//        }
//
//        audioEnabled = 1;
//    }
}

void handleSystemEvents() {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {

    if (event.type == SDL_QUIT) {
      mBufferedCommand = kCommandQuit;
      return;
    }

    if (event.type == SDL_KEYDOWN) {

      SDLKey key = event.key.keysym.sym;




      switch(event.key.keysym.scancode){
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

void graphicsShutdown() {
  SDL_Quit();

  releaseBitmap(defaultFont);

  texturesUsed = 0;
}

void flipRenderer() {

  SDL_GL_SwapBuffers();
  
#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif


}
