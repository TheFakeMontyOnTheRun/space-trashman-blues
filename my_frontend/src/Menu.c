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

const uint8_t shapeIndex[] = {14};
const uint8_t shape0[] = {
        6, 2,
        40, 63,
        42, 40,
        54, 39,
        54, 65,
        42, 66,
        40, 63
};
const uint8_t shape1[] = {
        7, 2,
        32, 102,
        24, 112,
        21, 139,
        59, 139,
        59, 97,
        41, 94,
        24, 92
};
const uint8_t shape2[] = {
        8, 2,
        35, 115,
        42, 116,
        59, 116,
        59, 139,
        32, 139,
        35, 132,
        35, 115,
        37, 90
};
const uint8_t shape3[] = {
        10, 2,
        36, 64,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        59, 106,
        42, 110,
        35, 96,
        35, 91,
        36, 64
};
const uint8_t shape4[] = {
        7, 2,
        35, 91,
        38, 74,
        59, 73,
        59, 96,
        57, 99,
        42, 105,
        35, 91
};
const uint8_t shape5[] = {
        8, 2,
        59, 38,
        51, 34,
        38, 42,
        35, 65,
        39, 78,
        50, 81,
        59, 74,
        59, 38
};
const uint8_t shape6[] = {
        9, 2,
        36, 71,
        35, 91,
        42, 105,
        57, 99,
        59, 95,
        58, 74,
        50, 80,
        39, 77,
        36, 67
};
const uint8_t shape7[] = {
        8, 2,
        54, 88,
        55, 92,
        56, 95,
        51, 104,
        46, 99,
        45, 97,
        50, 91,
        55, 85
};
const uint8_t shape8[] = {
        8, 2,
        41, 78,
        41, 85,
        43, 97,
        47, 98,
        54, 92,
        57, 75,
        50, 81,
        41, 78
};
const uint8_t shape9[] = {
        6, 2,
        39, 43,
        45, 40,
        58, 45,
        58, 39,
        49, 35,
        39, 43
};
const uint8_t shape10[] = {
        6, 2,
        36, 64,
        39, 75,
        40, 51,
        39, 49,
        38, 43,
        36, 64
};
const uint8_t shape11[] = {
        6, 2,
        50, 78,
        53, 53,
        56, 52,
        59, 47,
        59, 72,
        50, 78
};
const uint8_t shape12[] = {
        7, 2,
        39, 104,
        37, 100,
        31, 101,
        26, 115,
        29, 109,
        39, 104,
        52, 93
};
const uint8_t* shapes[] = {
        &shapeIndex[0],
        &shape0[0],
        &shape1[0],
        &shape2[0],
        &shape3[0],
        &shape4[0],
        &shape5[0],
        &shape6[0],
        &shape7[0],
        &shape8[0],
        &shape9[0],
        &shape10[0],
        &shape11[0],
        &shape12[0]
};


void drawGraphic(const uint8_t **graphic) {
  uint8_t nshapes = graphic[0][0];
  for (int s = 1; s < nshapes; ++s) {
    const uint8_t *shape = &graphic[s][0] + 2;
    uint8_t npoints = shapes[s][0];
    uint8_t colour = shapes[s][1];

    for (int c = 0; c < npoints - 1; ++c) {
      drawLine(shape[2 * c], shape[(2 * c) + 1], shape[(2 * c) + 2], shape[(2 * c) + 3], 2);
    }
    drawLine(shape[ 2 * npoints - 2], shape[ 2 * npoints - 1], shape[0], shape[1], 2);
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
    clearTextScreen();
    writeStr(1, 17, message);
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
