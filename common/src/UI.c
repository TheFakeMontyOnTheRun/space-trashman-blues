/*
* Created by Daniel Monteiro on 2019-08-02.
*/
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Enums.h"
#include "Common.h"
#include "Core.h"
#include "FixP.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "CActor.h"
#include "Mesh.h"
#include "Renderer.h"
#include "UI.h"
#include "SoundSystem.h"

void drawMenuBackground(void) {
    fillRect(0, 0, (XRES_FRAMEBUFFER), (YRES_FRAMEBUFFER), getPaletteEntry(0xFF6cb1a3), FALSE);
}

void
drawWindowWithOptions(const int x,
                      const int y,
                      const unsigned int dx,
                      const unsigned int dy,
                      const char *title,
                      const char **options,
                      uint8_t optionsCount,
                      uint8_t selectedOption) {
    int c;

    drawWindow(x,
               y,
               dx,
               dy,
               title);

    for (c = 0; c < optionsCount; ++c) {

        int isCursor = (selectedOption == c);

        if (isCursor) {
            fillRect(x * 8 - 8,
                     (y + 2 + c) * 8 - 8,
                     dx * 8,
                     8,
                     getPaletteEntry(0xFF000000),
                     FALSE);
        }

        drawTextAt(x + 1,
                   y + 2 + c,
                   &options[c][0],
                   isCursor ? getPaletteEntry(0xFFFFFFFF) : getPaletteEntry(0xFF000000));
    }
}

void
drawWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title) {

    fillRect((x) * 8, (y) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000), TRUE);
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000));
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, 8, getPaletteEntry(0xFF000000), FALSE);

    if (title != NULL) {
        drawTextAt(x + 1, y, title, getPaletteEntry(0xFFFFFFFF));
    }
}

int
drawAppearingWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                    long remainingTime) {

    if (remainingTime > 256) {
        return 0;
    }

    if (remainingTime > 0) {

        int middleX = x + (dx / 2);
        int middleY = y + (dy / 2);

        int invertedProgression = ((256 - (remainingTime)) / 32) * 32;
        int lerpPositionX = lerpInt(middleX, x, invertedProgression, 256);
        int lerpPositionWidth = lerpInt(0, dx, invertedProgression, 256);
        int lerpPositionY = lerpInt(middleY, y, invertedProgression, 256);
        int lerpPositionHeight = lerpInt(0, dy, invertedProgression, 256);

        drawRect(lerpPositionX * 8,
                 lerpPositionY * 8,
                 lerpPositionWidth * 8,
                 lerpPositionHeight * 8, getPaletteEntry(0xFF000000));

        return 0;
    }

    fillRect((x) * 8, (y) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000), TRUE);
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000));
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, 8, getPaletteEntry(0xFF000000), FALSE);
    drawTextAt(x + 1, y, title, getPaletteEntry(0xFFFFFFFF));

    return 1;
}

void
drawTextWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
               const char *content) {
    drawWindow(x, y, dx, dy, title);
    drawTextAt(x + 1, y + 2, content, getPaletteEntry(0xFF000000));
}

void
drawImageWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                struct Bitmap *content) {
    fillRect((x) * 8, (y) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000), TRUE);
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFFFFFFFF), FALSE);
    drawBitmap((x - 1) * 8, (y) * 8, content, TRUE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, getPaletteEntry(0xFF000000));
    fillRect((x - 1) * 8, (y - 1) * 8, dx * 8, 8, getPaletteEntry(0xFF000000), FALSE);
    drawTextAt(x + 1, y, title, getPaletteEntry(0xFFFFFFFF));
}

void updateMap(void) {
    int x, z;
    struct WorldPosition visPos = *getPlayerPosition();

    fillRect(XRES + 8 + (10 * 4) + (4 * -10), 2 + 8 + (8 * 4) + (4 * -8), 4 * 20, 4 * 16,
             getPaletteEntry(0xFF000066), FALSE);

    for (z = -8; z < 8; ++z) {
        for (x = -10; x < 10; ++x) {
            if (isPositionAllowed(visPos.x + x, visPos.y + z)) {
                fillRect(XRES + 8 + (10 * 4) + (4 * x), 2 + 8 + (8 * 4) + (4 * z), 4, 4, getPaletteEntry(0xFF000099),
                         FALSE);
            }
        }
    }

    fillRect(XRES + 8 + (10 * 4), 2 + 8 + (8 * 4), 4, 4, getPaletteEntry(0xFF0000FF), FALSE);
}

void redrawHUD(void) {
    int line = 0;
    struct ObjectNode *head;
    int c;
    struct Item *itemPtr;
    drawTextAtWithMargin(1, 1, XRES, thisMissionName, getPaletteEntry(0xFFFFFFFF));

    drawTextAt(1 + (XRES / 8), 1, " Map:", getPaletteEntry(0xFFFFFFFF));

#ifndef TILED_BITMAPS
    if (mapTopLevel != NULL) {
        drawBitmap(XRES, 72, mapTopLevel, 0);
    }
#else
    if (mapTopLevel[0] != NULL) {
        for (c = 0; c < 8; ++c) {
            drawBitmap(((c & 3) * 32), 8 + (c >> 2) * 32, mapTopLevel[c], 1);
        }
    }
#endif

    /* draw current item on the corner of the screen */
    head = getPlayerItems();

    while (head != NULL) {
        itemPtr = getItem(head->item);
        if (itemPtr != NULL) {
            if (line == currentSelectedItem) {
                char textBuffer[255];
                sprintf(&textBuffer[0], "%s", itemPtr->name);
                textBuffer[14] = 0;

	     /*
	        drawBitmapRaw(XRES + 8, 199 - 32 - 16 - 16, 32, 32,
		itemSprites[itemPtr->index]->rotations[0], 1);
	     */
                drawTextAtWithMarginWithFiltering(2 + ((XRES) / 8), 23, XRES_FRAMEBUFFER, itemPtr->name,
                                                  itemPtr->active ? getPaletteEntry(0xFFAAAAAA) : getPaletteEntry(
                                                          0xFFFFFFFF), '\n');
            }
            ++line;
        }
        head = head->next;
    }
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

void drawGraphic(const uint8_t *graphic) {
    const uint8_t *ptr = graphic;
    int buffer[6];

    while (*ptr) {
        uint8_t c;
        const uint8_t npoints = *ptr++;
        const uint8_t r = *ptr++;
        const uint8_t g = *ptr++;
        const uint8_t b = *ptr++;
        const FramebufferPixelFormat colour = getPaletteEntry( 0xFF000000 + (b << 16) + (g << 8) + r);
        const uint8_t *shape = ptr;
        int centerX = 0;
        int centerY = 0;

        if (npoints > 3) {
            for (c = 0; c < npoints; ++c) {
                centerX += shape[2 * c];
                centerY += shape[(2 * c) + 1];
            }

            centerX /= npoints;
            centerY /= npoints;

            buffer[4] = centerX;
            buffer[5] = centerY;

            for (c = 0; c < npoints - 1; ++c) {

                buffer[0] = shape[(2 * c) + 0];
                buffer[1] = shape[(2 * c) + 1];
                buffer[2] = shape[(2 * c) + 2];
                buffer[3] = shape[(2 * c) + 3];

                fillTriangle(&buffer[0], colour);
            }
            ptr += 2 * npoints;
        } else if (npoints == 3) {
            buffer[0] = shape[0];
            buffer[1] = shape[1];
            buffer[2] = shape[2];
            buffer[3] = shape[3];
            buffer[4] = shape[4];
            buffer[5] = shape[5];

            fillTriangle(&buffer[0], colour);

            ptr += 2 * npoints;
        } else if (npoints == 2) {
            drawLine(shape[0], shape[1], shape[2], shape[3], colour);
            ptr += 2 * npoints;
        } else if (npoints == 1) {
            fillRect(shape[0], shape[1], 1, 1, colour, FALSE);
            ptr += 2 * npoints;
        }
    }
}

void clearScreen(void) {
    fillRect( 0, 0, XRES_FRAMEBUFFER, YRES_FRAMEBUFFER, getPaletteEntry(0xFF000000), FALSE);
}
