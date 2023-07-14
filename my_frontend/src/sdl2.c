#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <Core.h>
#include <assert.h>

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "KeyboardUI.h"
#include "Menu.h"
#include "SDL.h"

extern struct ObjectNode *focusedItem;
extern struct ObjectNode *roomItem;
extern int accessGrantedToSafe;
SDL_Window *window;
SDL_Renderer *renderer;

uint8_t mBufferedCommand;
uint32_t palette[16];
uint8_t framebuffer[128 * 128];

void graphicsPut(uint8_t x, uint8_t y) {

    assert(x >= 0);
    assert(x < 128);
    assert(y >= 0);
    assert(y < 128);

    framebuffer[(128 * y) + x] = 1;
#ifdef PUTAFLIP
    graphicsFlush();
    SDL_Delay(100);
#endif
}

void clearTextScreen(void) {
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
    memset(framebuffer, 0, 128 * 128);
}

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour) {

}

uint8_t *realPut(uint16_t x, uint8_t y, uint8_t colour, uint8_t *ptr) {}

void printSituation(void) {
    struct ObjectNode *roomItems;
    struct ObjectNode *playerObjects = getPlayerItems();
    puts("---------------");
    puts("\nPlayer items:");

    while (playerObjects != NULL) {
        struct Item *item = getItem(playerObjects->item);

        printf("%c%c%s\n", (playerObjects == focusedItem) ? '>' : ' ', item->active ? '*' : '-', item->name);

        playerObjects = playerObjects->next;
    }

    puts("\nItems in room:");

    roomItems = getRoom(getPlayerRoom())->itemsPresent->next;

    while (roomItems != NULL) {
        struct Item *item = getItem(roomItems->item);

        printf("%c%c%s\n", (roomItems == roomItem) ? '>' : ' ', item->active ? '*' : '-', item->name);

        roomItems = roomItems->next;
    }
}

void clearScreen(void) {}

uint8_t getKey(void) {
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
                    mBufferedCommand = ' ';
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

void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX, uint8_t fg, uint8_t bg) {
    puts(text);
}

void init(void) {
    initKeyboardUI();

    mBufferedCommand = '.';
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    memset(framebuffer, 5, 128 * 128);
    window =
            SDL_CreateWindow("Derelict 8-bits SDL2 test", SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, 256, 256, SDL_WINDOW_SHOWN);

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
    SDL_Rect rect;
    uint32_t pixel;
    int x, y, r, g, b;

    rect.x = 0;
    rect.y = 0;
    rect.w = 256;
    rect.h = 256;

    SDL_SetRenderDrawColor(renderer, 0xFF,
                           0xFF,
                           0xFF, 255);
    SDL_RenderFillRect(renderer, &rect);

    for (y = 0; y < 128; ++y) {
        for (x = 0; x < 128; ++x) {
            int index = framebuffer[(128 * y) + x];
            rect.x = 1 + 2 * x;
            rect.y = 1 + 2 * y;
            rect.w = 2;
            rect.h = 2;


            if (index < 0 || index >= 16) {
                continue;
            }

            pixel = palette[index];

            r = (pixel & 0x00FF0000) >> 16;
            g = ((pixel & 0x0000FF00) >> 8);
            b = ((pixel & 0x000000FF));

            SDL_SetRenderDrawColor(renderer, r,
                                   g,
                                   b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}

void graphicsFlush(void) {
    flipRenderer();
    clearGraphics();
}