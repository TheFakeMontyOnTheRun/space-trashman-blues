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
SDL_Renderer *renderer;
int snapshotSignal = '.';

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

void graphicsInit() {
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
					useDither = FALSE;
					visibilityCached = FALSE;
					break;
				case SDLK_k:
					useDither = TRUE;
					visibilityCached = FALSE;
					break;


				case SDLK_LEFT:
					snapshotSignal = kCommandLeft;
					visibilityCached = FALSE;
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
					visibilityCached = FALSE;
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
					visibilityCached = FALSE;
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
}

void graphicsShutdown() {
	SDL_Quit();

	releaseBitmap(defaultFont);

	texturesUsed = 0;
}

void flipRenderer() {
	SDL_Rect rect;
	int x, y;

	if (!enableSmoothMovement || turnTarget == turnStep || (snapshotSignal != '.')) {

		uint8_t *pixelPtr = &framebuffer[0];

		for (y = dirtyLineY0; y < dirtyLineY1; ++y) {
			for (x = 0; x < 320; ++x) {
				uint32_t pixel;

				rect.x = 2 * x;
				rect.y = (24 * y) / 10;
				rect.w = 2;
				rect.h = 3;

				pixel = palette[framebuffer[(320 * y) + x]];

				SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
									   ((pixel & 0x0000FF00) >> 8) - 0x18,
									   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
				SDL_RenderFillRect(renderer, &rect);

				++pixelPtr;
			}
		}

		mBufferedCommand = snapshotSignal;
		snapshotSignal = '.';
		memcpy(previousFrame, framebuffer, 320 * 200);
	} else if (turnStep < turnTarget) {

		for (y = dirtyLineY0; y < dirtyLineY1; ++y) {
			for (x = 0; x < 320; ++x) {
				uint8_t index;

				if (x < XRES) {

					if (x >= turnStep) {
						index = previousFrame[(320 * y) - turnStep + x];
					} else {
						index = framebuffer[(320 * y) + x - (320 - XRES) - turnStep];
					}

				} else {
					index = framebuffer[(320 * y) + x];
				}

				uint32_t pixel = palette[index];

				rect.x = 2 * x;
				rect.y = (24 * y) / 10;
				rect.w = 2;
				rect.h = 3;

				SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
									   ((pixel & 0x0000FF00) >> 8) - 0x18,
									   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
				SDL_RenderFillRect(renderer, &rect);
			}
		}

		turnStep += 20;
	} else {

		uint8_t *pixelPtr = &framebuffer[0];

		for (y = dirtyLineY0; y < dirtyLineY1; ++y) {
			for (x = 0; x < 320; ++x) {
				uint8_t index;

				if (x < XRES) {

					if (x >= turnStep) {
						index = framebuffer[(320 * y) - turnStep + x];
					} else {
						index = previousFrame[(320 * y) + x - (320 - XRES) - turnStep];
					}

				} else {
					index = framebuffer[(320 * y) + x];
				}

				uint32_t pixel = palette[index];

				rect.x = 2 * x;
				rect.y = (24 * y) / 10;
				rect.w = 2;
				rect.h = 3;

				SDL_SetRenderDrawColor(renderer, (pixel & 0x000000FF) - 0x38,
									   ((pixel & 0x0000FF00) >> 8) - 0x18,
									   ((pixel & 0x00FF0000) >> 16) - 0x10, 255);
				SDL_RenderFillRect(renderer, &rect);
				++pixelPtr;
			}
		}

		turnStep -= 20;
	}


	SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
	SDL_Delay(1000 / 60);
#endif
}

void clearRenderer() {}
