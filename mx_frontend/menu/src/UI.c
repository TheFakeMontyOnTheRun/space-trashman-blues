/*
* Created by Daniel Monteiro on 2019-08-02.
*/
#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include <stdio.h>
#include <string.h>

#include "Core.h"

#include "Enums.h"
#include "FixP.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "FixP.h"
#include "Vec.h"
#include "CActor.h"
#include "MapWithCharKey.h"
#include "Common.h"
#include "Vec.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "MapWithCharKey.h"
#include "CTile3DProperties.h"
#include "CRenderer.h"
#include "VisibilityStrategy.h"


#include "UI.h"

void
drawWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title) {

    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 255, FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 255);
}

int
drawAppearingWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char * title, long remainingTime) {

    if (remainingTime > 256) {
        return 0;
    }

   if (remainingTime > 0) {

        int middleX = x + (dx / 2);
        int middleY = y + (dy / 2);

        int invertedProgression = ((256 - (remainingTime)) / 32) * 32;
        int lerpPositionX = lerpInt( middleX, x, invertedProgression, 256);
        int lerpPositionWidth = lerpInt( 0, dx, invertedProgression, 256);
        int lerpPositionY = lerpInt(middleY, y, invertedProgression, 256);
        int lerpPositionHeight = lerpInt( 0, dy, invertedProgression, 256);

        drawRect(lerpPositionX * 8,
                 lerpPositionY * 8,
                 lerpPositionWidth * 8,
                 lerpPositionHeight * 8, 0);

        return 0;
    }

    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 255, FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 255);

    return 1;
}

void
drawTextWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title,
               const char *__restrict__ content) {
    drawWindow(x, y, dx, dy, title);
    drawTextAtWithMargin(x + 1, y + 2, (x + dx - 2) * 8, content, 0);
}

void
drawImageWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title,
                const struct Bitmap *__restrict__ content) {
    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 255, FALSE);
    drawBitmap((x - 1) * 8, (y) * 8, content, TRUE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 255);
}

void redrawHUD(void) {
    int line = 0;
    struct ObjectNode *head;
    struct WorldPosition visPos = *getPlayerPosition();
    int x, z;
    struct Item* itemPtr;
    fill(XRES, 0, XRES_FRAMEBUFFER - XRES, YRES, 0, FALSE);
    drawTextAt(1 + (XRES / 8), 1, " Map:", 255);

    if (mapTopLevel != NULL) {
        drawBitmap(XRES, 72, mapTopLevel, 0);
    }

    //draw current item on the corner of the screen
    head = getPlayerItems();

    while (head != NULL) {
        itemPtr = getItem(head->item);
        if (itemPtr != NULL) {
            if (line == currentSelectedItem) {
                char textBuffer[255];
                sprintf(&textBuffer[0], "%s", itemPtr->name);
                textBuffer[14] = 0;

                int itemIndex = itemPtr->index;

                // lazy loading the item sprites
                // we can't preload it because...reasons on the NDS
                // perhaps some state machine issue? IDK. Placing this here works better for the NDS.
                if (itemSprites[itemIndex] == NULL) {
                    char buffer[64];
                    sprintf(&buffer[0], "%s.img", itemPtr->name);
                    itemSprites[itemIndex] = loadBitmap(&buffer[0]);
                }

                drawBitmapRaw(XRES + 8, YRES- 16 - 16, 16, 16, itemSprites[itemIndex]->data, 1);

                drawTextAtWithMarginWithFiltering(2 + ((XRES) / 8), ((YRES_FRAMEBUFFER / 8) + 1) - 2, 311, itemPtr->name,
                                     itemPtr->active ? 192 : 255, '\n');
            }
            ++line;
        }
        head = head->next;
    }

    fill(XRES + 8 + (10 * 4) + (4 * -10), 2 + 8 + (8 * 4) + (4 * -8), 4 * 20, 4 * 16,
         64, FALSE);

    for (z = -8; z < 8; ++z) {
        for (x = -10; x < 10; ++x) {
            if (isPositionAllowed(visPos.x + x, visPos.y + z)) {
                fill(XRES + 8 + (10 * 4) + (4 * x), 2 + 8 + (8 * 4) + (4 * z), 4, 4,192, FALSE);
            }
        }
    }

    fill(XRES + 8 + (10 * 4), 2 + 8 + (8 * 4), 4, 4,32, FALSE);
}
