#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <Core.h>
#include <assert.h>


#include "SDL.h"

int cursorPosition = 0;
extern struct ObjectNode* focusedItem;
extern struct ObjectNode* roomItem;
extern int accessGrantedToSafe;
SDL_Window *window;
SDL_Renderer *renderer;

#define TRANSPARENCY_COLOR 17

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[160 * 200];

void graphicsFlush();

void nextItemInHand();

void useItemInHand();

void nextItemInRoom();

void interactWithItemInRoom();

void pickOrDrop();

void pickItem();

void graphicsPut(int16_t x, int16_t y) {
	if (x < 0 ) {
		x = 0;
	}

	if (x >= 128) {
		x = 127;
	}

	if (y < 0 ) {
		y = 0;
	}

	if (y >= 128) {
		y = 127;
	}



	framebuffer[(160 * y) + x] = 1;
#ifdef PUTAFLIP
    graphicsFlush();
    SDL_Delay(100);
#endif
}

void vLine(int16_t x0, int16_t y0, int16_t y1, uint8_t shouldStipple) {

	if (x0 <0 ) {
		return;
	}

    int16_t y;
    int16_t _y0 = y0;
    int16_t _y1 = y1;

    if (y0 > y1) {
        _y0 = y1;
        _y1 = y0;
    }

	if (_y0 < 0 ) {
		_y0 = 0;
	}

	if (_y1 >= 128) {
		_y1 = 127;
	}

	uint8_t stipple;
	uint8_t colour;

	if (shouldStipple <= 7) {
		colour = shouldStipple;
		shouldStipple = 0;
		stipple = 1;
	} else {
		colour = shouldStipple - 8;
		shouldStipple = 1;
		stipple = (x0 & 1);
	}

    for ( y = _y0; y <= _y1; ++y) {
		if (shouldStipple) {
			stipple = !stipple;
		}

		if (stipple) {
			framebuffer[(160 * y) + x0] = colour;
		}
    }
}

void shutdownGraphics() {
    SDL_Quit();
}

void showMessage(const char* mesg) {
    puts(mesg);
}

void drawWindow(int tx, int ty, int tw, int th, const char* title ) {}

void clearGraphics() {
    memset(framebuffer, 0, 160 * 200);
}

void writeStr(uint8_t nColumn, uint8_t nLine, char *str, uint8_t fg, uint8_t bg) {
    puts(str);
}

void printSituation() {
    struct ObjectNode *playerObjects = getPlayerItems();
    puts("---------------");
    puts("\nPlayer items:");

    while( playerObjects != NULL ) {
        struct Item *item = getItem(playerObjects->item);

        printf("%c%c%s\n", (playerObjects == focusedItem) ? '>' : ' ', item->active ? '*' : '-', item->name );

        playerObjects = playerObjects->next;
    }

    puts("\nItems in room:");

    struct ObjectNode *roomItems = getRoom(getPlayerRoom())->itemsPresent->next;

    while( roomItems != NULL ) {
        struct Item *item = getItem(roomItems->item);

        printf("%c%c%s\n", (roomItems == roomItem) ? '>' : ' ',item->active ? '*' : '-', item->name );

        roomItems = roomItems->next;
    }
}

void dropItem();

void pickItem();

void clearScreen() {}


uint8_t getKey() {
    SDL_Event event;

    mBufferedCommand = '.';

    while (SDL_PollEvent(&event)) {

        if (event.type == SDL_QUIT) {

            mBufferedCommand = 'q';
        }

        if (event.type == SDL_KEYUP) {

            switch (event.key.keysym.sym) {
                case SDLK_RETURN:
                case SDLK_z:
                    mBufferedCommand = 'a';
                    break;

                case SDLK_ESCAPE:
                case SDLK_q:
                    mBufferedCommand = 'l';
                    break;

                case SDLK_SPACE:
                    printSituation();
                    break;

                case SDLK_KP_7:
                    mBufferedCommand = '7';
                    break;

                case SDLK_KP_8:
                    mBufferedCommand = '8';
                    break;


                case SDLK_KP_4:
                    mBufferedCommand = '4';
                    break;

                case SDLK_KP_5:
                    mBufferedCommand = '5';
                    break;

                case SDLK_KP_9:
                    mBufferedCommand = '9';
                    break;

                case SDLK_KP_6:
                    mBufferedCommand = '6';
                    break;


                case SDLK_s:
                    break;
                case SDLK_d:
                    break;
                case SDLK_v:
                    break;
                case SDLK_b:
                    break;
                case SDLK_j:
                    break;
                case SDLK_k:
                    break;
                case SDLK_x:
                    mBufferedCommand = 'd';
                    break;
                case SDLK_c:
                    break;
                case SDLK_e:
                    break;

                case SDLK_LEFT:
                    mBufferedCommand = 'q';
                    break;
                case SDLK_RIGHT:
                    mBufferedCommand = 'e';
                    break;
                case SDLK_UP:
                    mBufferedCommand = 'w';
                    break;
                case SDLK_DOWN:
                    mBufferedCommand = 's';
                    break;

                default:
                    return '.';
            }
        }
    }

    return mBufferedCommand;
}

void sleepForMS() {
}

void init() {
    int r, g, b;
    mBufferedCommand = '.';
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    memset(framebuffer, 0, 160 * 200);
    window =
            SDL_CreateWindow("Derelict 8-bits SDL2 test", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, 0);

    palette[0] = 0xff000000;
    palette[1] = 0xff0000ff;
    palette[2] = 0xff00ff00;
    palette[3] = 0xff00ffff;
    palette[4] = 0xffff0000;
    palette[5] = 0xffff00ff;
    palette[6] = 0xffffff00;
    palette[7] = 0xffffffff;
    palette[8] = 0xff000000;
    palette[9] = 0xff00007f;
    palette[10] = 0xff007f00;
    palette[11] = 0xff007f7f;
    palette[12] = 0xff7f0000;
    palette[13] = 0xff7f007f;
    palette[14] = 0xff7f7f00;
    palette[15] = 0xff7f7f7f;

#ifdef __EMSCRIPTEN__
    enterFullScreenMode ();
#endif
}


void titleScreen() {
    int keepGoing = 1;
    clearGraphics();

    writeStr(1, 1, "Space Mare Imperium:", 2, 0);
    writeStr(1, 2, "     Derelict", 2, 0);
    writeStr(1, 4, "by Daniel Monteiro", 2, 0);
    writeStr(1, 6, "  Press B button ", 2, 0);
    writeStr(1, 7, "    to start", 2, 0);

    while (keepGoing) {
        if (getKey() != '.') {
            keepGoing = 0;
        }
    }

    clearScreen();
}

void flipRenderer() {
    SDL_Rect rect;
    uint32_t pixel;
    int x, y;

    rect.x = 0;
    rect.y = 0;
    rect.w = 259;
    rect.h = 309;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);

    for (y = 0; y < 128; ++y) {
        for (x = 0; x < 128; ++x) {

            rect.x = 1 + 2 * x;
            rect.y = 1 + (24 * y) / 10;
            rect.w = 2;
            rect.h = 3;
            int index = framebuffer[(160 * y) + x];

            if (index < 0 || index >= 16) {
                continue;
            }

            pixel = palette[index];

            int r = (pixel & 0x00FF0000) >> 16;
            int g = ((pixel & 0x0000FF00) >> 8);
            int b = ((pixel & 0x000000FF));

            SDL_SetRenderDrawColor(renderer, r,
                                   g,
                                   b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);

#ifndef __EMSCRIPTEN__
    SDL_Delay(1000 / 60);
#endif
}

void graphicsFlush() {
    flipRenderer();
}


void HUD_initialPaint() {
}

void HUD_refresh() {
}


