/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdlib.h>
#include <stdint.h>

#include "Enums.h"
#include "UI.h"
#include "Core.h"
#include "Derelict.h"
#include "Renderer.h"

extern uint8_t playerLocation;

extern const struct CellPattern patterns[127];

extern int8_t map[32][32];

const uint8_t shapes[] = {
        7, 2,
        28, 103,
        16, 140,
        54, 140,
        55, 107,
        38, 111,
        32, 100,
        32, 100,
        7, 2,
        30, 116,
        32, 109,
        40, 117,
        54, 117,
        54, 140,
        28, 140,
        28, 140,
        10, 2,
        32, 64,
        31, 92,
        38, 106,
        53, 99,
        54, 96,
        54, 107,
        38, 111,
        31, 97,
        31, 92,
        31, 92,
        8, 2,
        54, 39,
        44, 38,
        34, 43,
        31, 66,
        35, 79,
        46, 82,
        54, 74,
        54, 74,
        9, 2,
        31, 72,
        31, 92,
        38, 106,
        53, 99,
        54, 96,
        54, 75,
        45, 81,
        35, 78,
        31, 68,
        6, 2,
        37, 79,
        39, 98,
        43, 99,
        53, 76,
        46, 81,
        46, 81,
        5, 2,
        31, 65,
        35, 76,
        36, 52,
        34, 44,
        34, 44,
        5, 2,
        46, 79,
        49, 54,
        54, 48,
        54, 73,
        54, 73,
        4, 2,
        38, 111,
        35, 78,
        31, 97,
        31, 97,
        4, 2,
        31, 97,
        35, 78,
        32, 68,
        32, 68,
        4, 2,
        38, 111,
        46, 81,
        55, 107,
        55, 107,
        0 };

void drawGraphic(const uint8_t *graphic) {

  const uint8_t* ptr = graphic;

  while (*ptr) {
	int c;
    const uint8_t npoints = *ptr; 
    const uint8_t colour = *(ptr + 1);
    ptr += 2;
    const uint8_t *shape = ptr;
    
    for (c = 0; c < npoints - 1; ++c) {
      drawLine(shape[2 * c], shape[(2 * c) + 1], shape[(2 * c) + 2], shape[(2 * c) + 3], colour);
      ptr += 2;
    }
    drawLine(shape[ 2 * npoints - 2], shape[ 2 * npoints - 1], shape[0], shape[1], colour);
    ptr += 2;
  }
}

void titleScreen(void) {
  uint8_t keepGoing = 1;
  clearScreen();

  drawGraphic(shapes);

#ifndef GAMEPAD
  writeStr(16, 1, "Sub Mare\nImperium:\nDerelict\nby\nDaniel Monteiro\nPress SPACE to\nstart ");
#else
  writeStr(16, 1, "Sub Mare\nImperium:\nDerelict\nby\nDaniel Monteiro\nPress Start!");
#endif

#ifdef SDLW
    /* Just to keep the OS happy */
    flushVirtualFramebuffer();
#endif


    while (keepGoing) {
        enum ECommand key = getInput();
        if (key != kCommandFire1) {
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
    clearTextScreen();
    writeStr(1, 17, message);
#ifndef GAMEPAD
    writeStr(1, 22, "Press SPACE button to continue");
#else
    writeStr(1, 22, "Press start to continue");
#endif

#ifdef SDLW
    /* Just to keep the OS happy */
    flushVirtualFramebuffer();
#endif

    while (keepGoing) {
        uint8_t key = getInput();
        if (key == ' ' || key == 'p') {
            keepGoing = 0;
        }
    }
    clearTextScreen();
    HUD_initialPaint();
}

void drawMap(void) {

    uint8_t x, y;

    if (playerLocation == 0) {
        return;
    }

    for (y = 0; y < 13; ++y) {
        for (x = 0; x < 13; ++x) {
            writeStr(17 + x, 1 + y, " ");
        }
    }

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {
            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                fillRect(128 + (x * 3) + 16, (y * 3) + 16, 128 + (x * 3) + 3 + 16, (y * 3) + 3 + 16, 15);
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
