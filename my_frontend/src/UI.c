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
#include "Engine.h"
#include "SoundSystem.h"

extern uint8_t playerLocation;

extern int8_t cursorPosition;

extern const struct CellPattern patterns[127];

extern int8_t map[32][32];

void drawGraphic(const uint8_t *graphic) {
    const uint8_t *ptr = graphic;

    while (*ptr) {
        uint8_t c;
        const uint8_t npoints = *ptr++;
        const uint8_t *shape = ptr;

        for (c = 0; c < npoints - 1; ++c) {
            drawLine(shape[2 * c], shape[(2 * c) + 1], shape[(2 * c) + 2], shape[(2 * c) + 3], 2);
        }
        drawLine(shape[2 * npoints - 2], shape[2 * npoints - 1], shape[0], shape[1], 2);
        ptr += 2 * npoints;
    }
}

void drawTextAt(uint8_t _x, uint8_t y, const char *text, uint8_t colour) {
    writeStrWithLimit(_x, y, text, (XRES_FRAMEBUFFER / 8), colour, 0);
}

void showMessage(const char *message) {
    clearTextScreen();
    drawTextAt(1, 17, message, 1);
}

void drawMap(void) {

    uint8_t x, y;

    if (playerLocation == 0) {
        return;
    }

    for (y = 0; y < 12; ++y) {
        for (x = 0; x < 12; ++x) {
            drawTextAt(16 + x + 2, 2 + y, " ", 0);
        }
    }

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {

            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                fillRect(128 + (x * 3) + 16,
                         (y * 3) + 16,
                         128 + (x * 3) + 3 + 16,
                         (y * 3) + 3 + 16,
                         2,
                         0);
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
    }
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title, uint8_t colour) {
    uint16_t x0 = tx * 8;
    uint16_t x1 = (tx + tw) * 8;
    uint8_t y0 = ty * 8;
    uint8_t y1 = (ty + th) * 8;

    drawLine(x0, y0, x1, y0, colour);

    drawLine(x0, y1, x1, y1, colour);

    drawLine(x0, y0, x0, y1, colour);

    drawLine(x1, y0, x1, y1, colour);

    drawTextAt(tx + 1, ty + 1, title, 1);
}


void
drawWindowWithOptions(const uint8_t x,
                      const uint8_t y,
                      const uint8_t dx,
                      const uint8_t dy,
                      const char *title,
                      const char **options,
                      const uint8_t optionsCount,
                      const uint8_t selectedOption) {
    uint8_t c;

    if (options != NULL) {
        drawWindow(x - 1,
                   y - 1,
                   dx + 1,
                   dy + 1,
                   title,
                   2);
    }

    for (c = 0; c < optionsCount; ++c) {
        drawTextAt(x,
                   y + 2 + c,
                   (selectedOption == c) ? ">" : " ",
                   1);

        if (options != NULL) {
            drawTextAt(x + 1,
                       y + 2 + c,
                       &options[c][0],
                       1);
        }
    }
}

void
drawTextWindow(const uint8_t x, const uint8_t y, const uint8_t dx, const uint8_t dy, const char *title,
               const char *content) {
    drawWindow(x, y, dx, dy, title, 2);
    drawTextAt(x + 1, y + 2, content, 2);
}

enum EGameMenuState handleCursor(const enum EGameMenuState* options, uint8_t optionsCount, const enum ECommand cmd, enum EGameMenuState backState) {

    switch (cmd) {
        case kCommandBack:
            return backState;
        case kCommandUp:
            playSound(2);
            --cursorPosition;
            break;
        case kCommandDown:
            playSound(2);
            ++cursorPosition;
            break;
        case kCommandFire1:
        case kCommandFire2:
        case kCommandFire3:
            return options[cursorPosition];
    }

    if (cursorPosition >= optionsCount) {
        cursorPosition = optionsCount - 1;
    }

    if (cursorPosition < 0) {
        cursorPosition = 0;
    }

    return kResumeCurrentState;
}