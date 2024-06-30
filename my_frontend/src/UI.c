/*
   Created by Daniel Monteiro on 11/07/2023.
*/
#include <stdint.h>

#include "Enums.h"
#include "UI.h"
#include "Core.h"
#include "Renderer.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Common.h"

extern uint8_t playerLocation;

extern int8_t cursorPosition;

extern const struct CellPattern patterns[127];

extern int8_t map[32][32];

uint8_t waitForKey = 0;

uint8_t redrawMap;

uint8_t needsToRedrawHUD;

uint16_t scale2DVertex( uint16_t offset, uint16_t scale, const uint8_t *shape, uint16_t index) {
    return offset + ((scale * shape[index]) / 128);
}

void drawGraphic(uint16_t x, uint8_t  y, uint16_t dx, uint8_t dy, const uint8_t *graphic) {
    const uint8_t *ptr = graphic;

    while (*ptr) {
        uint8_t c;
        const uint8_t npoints = *ptr++;
#ifndef MONOCHROME_VECTORS
        ptr += 3; /* skip colours */
#endif
        const uint8_t *shape = ptr;

        for (c = 0; c < npoints - 1; ++c) {
            drawLine(scale2DVertex( x, dx, shape, (2 * c) + 0),
                     scale2DVertex( y, dy, shape, (2 * c) + 1),
                     scale2DVertex( x, dx, shape, (2 * c) + 2),
                     scale2DVertex( y, dy, shape, (2 * c) + 3),
                     2);
        }

        drawLine(scale2DVertex( x, dx, shape, 2 * npoints - 2),
                 scale2DVertex( y, dy, shape, 2 * npoints - 1),
                 scale2DVertex( x, dx, shape, 0),
                 scale2DVertex( y, dy, shape, 1),
                 2);

        ptr += 2 * npoints;
    }
}

void drawTextAt(uint8_t _x, uint8_t y, const char *text, uint8_t colour) {
    drawTextAtWithMargin(_x, y, (XRES_FRAMEBUFFER), text, colour);
}

void drawTextAtWithMargin(const int x, const int y, int margin, const char *text, const uint8_t colour) {
    drawTextAtWithMarginWithFiltering(x, y, margin, text, colour, '-');
}

void showMessage(const char *message) {
    uint8_t lines = countLines(message) + 3;
    drawTextWindow( 0, (YRES / 8) + 1, (XRES_FRAMEBUFFER / 8) - 1, lines + 1, "Press 2 to continue", message);
    waitForKey = 1;
}

void drawMap(void) {
    uint8_t x, y;

    if (!redrawMap) {
        return;
    }

    if (playerLocation == 0) {
        return;
    }

    redrawMap = 0;

    drawWindow((XRES_FRAMEBUFFER / 8) / 2,
               0,
               (XRES_FRAMEBUFFER / 8) / 2 - 1,
               (YRES_FRAMEBUFFER / 8) / 2 + 1,
               "Map");

    for (y = 0; y < 32; ++y) {
        for (x = 0; x < 32; ++x) {

            if (patterns[(map[y][x] & 127) - 32].blockMovement) {
                fillRect((XRES_FRAMEBUFFER / 2) + (x * 3) + 16,
                         (y * 3) + 15,
                         (XRES_FRAMEBUFFER / 2) + (x * 3) + 16 + 3,
                         (y * 3) + 3 + 16,
                         2,
                         0);
            }
        }
    }
}

void performAction(void) {
    const char *msg = NULL;
    switch (getGameStatus()) {
        case kBadVictory:
            msg = "Victory! Too bad you didn't survive";
            break;


        case kBadGameOver:
            msg = "You're dead! And so are the "
                        "other people on the path of "
                        "destruction faulty reactor";
            break;

        case kGoodVictory:
            msg = "Victory! You managed to destroy the ship and get out alive";
            break;

        case kGoodGameOver:
            msg = "You failed! While you're alive "
                        "you failed to prevent the worst "
                        "scenario and now EVERYBODY is "
                        "dead!)";
            break;
    }

    if (msg) {
        uint8_t lines = countLines(msg) + 3;
        clearScreen();
        drawTextWindow( 0, (YRES / 8) + 1, (XRES_FRAMEBUFFER / 8) - 1, lines + 1, "", msg);

        while (1);
    }
}

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title) {
    uint16_t x0 = tx * 8;
    uint16_t x1 = (tx + tw) * 8;
    uint8_t y0 = ty * 8;
    uint8_t y1 = (ty + th) * 8;
    uint8_t c, d;

    for (c = 0; c < th; ++c) {
        for (d = 0; d < tw; ++d) {
            drawTextAtWithMarginWithFiltering( tx + d, ty + c, XRES_FRAMEBUFFER, " ", 2, ' ');
        }
    }

    drawLine(x0, y0, x1, y0, 2);

    drawLine(x0, y1, x1, y1, 2);

    drawLine(x0, y0, x0, y1, 2);

    drawLine(x1, y0, x1, y1, 2);

    drawTextAt(tx + 1, ty, title, 1);
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

    if (firstFrameOnCurrentState || selectedOption == 0xFF) {
        drawWindow(x - 1,
                   y - 1,
                   dx + 1,
                   dy + 1,
                   title);
    }

    for (c = 0; c < optionsCount; ++c) {
        drawTextAt(x,
                   y + 1 + c,
                   (selectedOption == c) ? ">" : " ",
                   1);

        if (firstFrameOnCurrentState || selectedOption == 0xFF) {
            drawTextAt(x + 1,
                       y + 1 + c,
                       &options[c][0],
                       1);
        }
    }
}

void
drawTextWindow(const uint8_t x, const uint8_t y, const uint8_t dx, const uint8_t dy, const char *title,
               const char *content) {
    drawWindow(x, y, dx, dy, title);
    drawTextAtWithMargin(x + 1, y + 2, (x + dx - 1) * 8, content, 1);
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
