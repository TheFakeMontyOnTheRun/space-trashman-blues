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

const uint16_t index_shapes[1] = {
				  14
};

const uint16_t shape0[] = {
			   6, 1,
			   253, 240,
			   266, 154,
			   341, 149,
			   340, 246,
			   265, 250,
			   253, 240
};
const uint16_t shape1[] = {
			   7, 1,
			   203, 388,
			   156, 425,
			   132, 526,
			   372, 526,
			   372, 366,
			   262, 358,
			   152, 351
};
const uint16_t shape2[] = {
			   8, 1,
			   220, 435,
			   264, 441,
			   372, 441,
			   372, 526,
			   203, 526,
			   220, 499,
			   220, 435,
			   237, 343
};
const uint16_t shape3[] = {
			   10, 1,
			   228, 242,
			   224, 345,
			   267, 398,
			   364, 374,
			   372, 362,
			   372, 402,
			   267, 418,
			   222, 365,
			   223, 346,
			   228, 242
};
const uint16_t shape4[] = {
			   7, 2,
			   224, 345,
			   241, 280,
			   372, 278,
			   372, 366,
			   364, 374,
			   265, 400,
			   224, 345
};
const uint16_t shape5[] = {
			   8, 3,
			   372, 145,
			   321, 131,
			   240, 161,
			   223, 246,
			   248, 295,
			   316, 306,
			   372, 279,
			   372, 145
};
const uint16_t shape6[] = {
			   9, 1,
			   226, 269,
			   223, 344,
			   266, 399,
			   361, 374,
			   372, 362,
			   371, 280,
			   315, 304,
			   248, 293,
			   227, 254
};
const uint16_t shape7[] = {
			   8, 1,
			   344, 335,
			   349, 347,
			   354, 361,
			   326, 396,
			   289, 376,
			   287, 366,
			   317, 345,
			   347, 324
};
const uint16_t shape8[] = {
			   8, 1,
			   262, 297,
			   261, 323,
			   277, 370,
			   300, 373,
			   345, 350,
			   360, 285,
			   316, 306,
			   262, 297
};
const uint16_t shape9[] = {
			   6, 1,
			   245, 163,
			   288, 154,
			   370, 172,
			   371, 148,
			   313, 134,
			   245, 163
};
const uint16_t shape10[] = {
			    6, 1,
			    227, 242,
			    251, 285,
			    255, 195,
			    246, 187,
			    244, 165,
			    227, 242
};
const uint16_t shape11[] = {
			    6, 1,
			    318, 296,
			    337, 201,
			    356, 196,
			    372, 178,
			    372, 275,
			    318, 296
};
const uint16_t shape12[] = {
			    7, 2,
			    247, 395,
			    233, 380,
			    198, 385,
			    165, 437,
			    187, 412,
			    247, 395,
			    330, 352
};

const uint16_t *shapes[] = {
			    &index_shapes[0],
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

void drawGraphic(const uint16_t **graphic) {
  uint8_t nshapes = graphic[0][0];
  for (int s = 1; s < nshapes; ++s) {
    const uint16_t *shape = &graphic[s][0] + 2;
    uint8_t npoints = shapes[s][0];
    uint8_t colour = shapes[s][1];
    
    for (int c = 0; c < npoints - 1; ++c) {
      drawLine(shape[2 * c] / 4, shape[(2 * c) + 1] / 4, shape[(2 * c) + 2] / 4, shape[(2 * c) + 3] / 4, colour);
    }
    drawLine(shape[ 2 * npoints - 2] / 4, shape[ 2 * npoints - 1] / 4, shape[0] / 4, shape[1] / 4, colour);   
  } 
}

void titleScreen(void) {
  uint8_t keepGoing = 1;
  clearScreen();
  
  drawGraphic(shapes);
    
  writeStr(16, 1, "Sub Mare Imperium: Derelict");
  writeStr(16, 4, "by Daniel Monteiro");
  writeStr(16, 6, " Press SPACE to start ");
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
