#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
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

#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#ifdef __EMSCRIPTEN__
void enterFullScreenMode(void) {
	EmscriptenFullscreenStrategy s;
	memset(&s, 0, sizeof(s));
	s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
	s.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
	s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
	emscripten_enter_soft_fullscreen(0, &s);
}
#endif

FILE* demoFile;
SDL_Window *window;
SDL_Renderer *renderer;
int snapshotSignal = '.';
int recordingCommand = 0;
int playingDemo = 0;
int commandRepetitions = 0;
uint8_t lastCommand = 255;

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

void graphicsInit(void) {
	int r, g, b;

	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	window =
			SDL_CreateWindow("Sub Mare Imperium - Derelict", SDL_WINDOWPOS_CENTERED,
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
	enableSmoothMovement = TRUE;
}

void handleSystemEvents(void) {
	SDL_Event event;

	if (playingDemo) {
	  printf("%d, %d\n", commandRepetitions, mBufferedCommand);
	  if (!(commandRepetitions--)) {
	    fscanf(demoFile, "%d, %d\n", &commandRepetitions, &mBufferedCommand);
	    printf("read %d, %d\n", commandRepetitions, mBufferedCommand);	    
	  }
	  if (feof(demoFile)) {
	    playingDemo = 0;
	  }
	}
	
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
					visibilityCached = FALSE;
					needsToRedrawVisibleMeshes = TRUE;
                    needsToRedrawHUD = TRUE;
					break;
				case SDLK_x:
					mBufferedCommand = kCommandFire2;
					visibilityCached = FALSE;
					needsToRedrawVisibleMeshes = TRUE;
                    needsToRedrawHUD = TRUE;
					break;
			case SDLK_p:
			  commandRepetitions = 0;
			  mBufferedCommand = '.';
			  playingDemo = 1;
			  demoFile = fopen("demo.txt", "r");			  
			  break;
			case SDLK_r:
			  recordingCommand = 1; 
			  break;
			case SDLK_c:
					mBufferedCommand = kCommandFire3;
					visibilityCached = FALSE;
					needsToRedrawVisibleMeshes = TRUE;
                    needsToRedrawHUD = TRUE;
					break;
				case SDLK_v:
					mBufferedCommand = kCommandFire4;
					visibilityCached = FALSE;
					needsToRedrawVisibleMeshes = TRUE;
                    needsToRedrawHUD = TRUE;
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

				case SDLK_LEFT:
					snapshotSignal = kCommandLeft;
					if ((currentGameMenuState == kPlayGame ||
						 currentGameMenuState == kBackToGame) &&
						currentPresentationState == kWaitingForInput
							) {
						turnStep = 0;
						turnTarget = 200;
					}
					break;
				case SDLK_RIGHT:
					snapshotSignal = kCommandRight;
					if ((currentGameMenuState == kPlayGame ||
						 currentGameMenuState == kBackToGame) &&
						currentPresentationState == kWaitingForInput
							) {
						turnStep = 200;
						turnTarget = 0;
					}
					break;
				case SDLK_UP:
					mBufferedCommand = kCommandUp;
					break;
				case SDLK_1:
					enableSmoothMovement = TRUE;
					break;

				case SDLK_2:
					enableSmoothMovement = FALSE;
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

	if (recordingCommand) {
	  if (mBufferedCommand != lastCommand ) {
	    printf("%d, %d\n", commandRepetitions, mBufferedCommand);
	    lastCommand = mBufferedCommand;
	    commandRepetitions = 1;
	  } else {
	    commandRepetitions++;
	  }
	}	  
}

void graphicsShutdown(void) {
	SDL_Quit();

	releaseBitmap(defaultFont);

	texturesUsed = 0;
	
	if (recordingCommand) {
	  printf("%d, %d\n", commandRepetitions, mBufferedCommand);
	}
}

void flipRenderer(void) {
	SDL_Rect rect;
	int x, y;

    uint8_t newFrame[XRES_FRAMEBUFFER * YRES_FRAMEBUFFER];

    renderPageFlip(newFrame, framebuffer,
                   previousFrame, turnStep, turnTarget, 0);

    for (y = dirtyLineY0; y < dirtyLineY1; ++y) {
        for (x = 0; x < XRES_FRAMEBUFFER; ++x) {
            uint32_t pixel;

            rect.x = 2 * x;
            rect.y = (24 * y) / 10;
            rect.w = 2;
            rect.h = 3;

            pixel = palette[newFrame[(XRES_FRAMEBUFFER * y) + x]];

            SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
                                   ((pixel & 0x0000FF00) >> 8) - 0x18,
                                   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    mBufferedCommand = snapshotSignal;
    snapshotSignal = '.';

	SDL_RenderPresent(renderer);
}

void clearRenderer(void) {}
