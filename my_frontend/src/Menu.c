/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Menu.h"
#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"

extern uint8_t playerLocation;

extern const struct Pattern patterns[127];

extern int8_t map[32][32];


void titleScreen(void) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, "Sub Mare Imperium: Derelict");
    writeStr(1, 4, "by Daniel Monteiro");
    writeStr(1, 6, " Press SPACE to start ");
#ifdef SDLW
    /* Just to keep the OS happy */
    flushVirtualFramebuffer();
#endif


    while (keepGoing) {
        uint8_t key = getKey();
        if (key == ' ' || key == 'p') {
            keepGoing = 0;
        }
    }
    clearScreen();
}

void writeStr(uint8_t _x, uint8_t y, const char *text) {
    writeStrWithLimit(_x, y, text, (XRES_FRAMEBUFFER / 8), 2, 0);
}

void showMessage(const char *message) {
    uint8_t keepGoing = 1;
    clearScreen();

    writeStr(1, 1, message);
    writeStr(1, 22, "Press SPACE button to continue");

#ifdef SDLW
    /* Just to keep the OS happy */
    flushVirtualFramebuffer();
#endif

    while (keepGoing) {
        uint8_t key = getKey();
        if (key == ' ' || key == 'p') {
            keepGoing = 0;
        }
    }
    clearScreen();
}

void drawMap(void) {

    uint8_t x, y;

    if (playerLocation == 0) {
        return;
    }

    for (y = 0; y < 8; ++y) {
        writeStr(17, 1 + y, "         ");
    }

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                fillRect(136 + (x * 2), (y * 2) + 8, 136 + (x * 2) + 2, (y * 2) + 8 + 2, 15);
            }
        }
    }
}

void performAction(void) {
    switch (getGameStatus()) {
        case kBadVictory:
            showMessage("Victory! Too bad you didn't survive");
            while (1);

        case kBadGameOver:
            showMessage("You're dead! And so are the\n"
                        "other people on the path of\n"
                        "destruction faulty reactor");
            while (1);

        case kGoodVictory:
            showMessage("Victory! You managed to destroy the\nship and get out alive");
            while (1);

        case kGoodGameOver:
            showMessage("You failed! While you're alive\n"
                        "you failed to prevent the worst\n"
                        "scenario and now EVERYBODY is\n"
                        "dead!)");
            while (1);

        default:
        case kNormalGameplay:
            break;
    }
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title, uint8_t colour) {

    uint8_t x0 = tx * 8;
    uint8_t x1 = (tx + tw) * 8;
    uint8_t y0 = ty * 8;
    uint8_t y1 = (ty + th) * 8;

    drawLine(x0, y0, x1, y0, colour);

    drawLine(x0, y1, x1, y1, colour);

    drawLine(x0, y0, x0, y1, colour);

    drawLine(x1, y0, x1, y1, colour);

    writeStr(tx + 1, ty, title);
}