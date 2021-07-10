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

void drawWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title) {

    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 255, FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 255);
}

void drawTextWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title,
                    const char *__restrict__ content) {
    drawWindow(x, y, dx, dy, title);
    drawTextAt(x + 1, y + 2, content, 0);
}

void drawImageWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *__restrict__ title,
                     const struct Bitmap *__restrict__ content) {
    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 255, FALSE);
    drawBitmap((x - 1) * 8, (y) * 8, content, TRUE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 255);
}

void redrawHUD() {
    int line = 0;
    struct ObjectNode *head;
    int playerHealth = getPlayerHealth();
    struct WorldPosition visPos = *getPlayerPosition();
    int x, z;

    fill(XRES, 0, 320 - XRES, 200, 0, FALSE);
    drawTextAt(1 + (XRES / 8), 1, " Map:", 255);

    if (mapTopLevel != NULL) {
        drawBitmap(208, 72, mapTopLevel, 0);
    }

    drawTextAt(1 + 1 + (XRES / 8), 17, "Health:", 255);
    drawRect(XRES + 8, 138, 100, 8, 255);
    fill(XRES + 8, 138, playerHealth, 8, 255, TRUE);


    //draw current item on the corner of the screen
    head = getPlayerItems();

    while (head != NULL) {
        if (head->item != NULL) {
            if (line == currentSelectedItem) {
                char textBuffer[255];
                sprintf(&textBuffer[0], "%s", head->item->description);
                textBuffer[14] = 0;

                drawBitmapRaw(XRES + 8, 199 - 32 - 16, 32, 32, itemSprites[head->item->index]->rotations[0], 1);

                drawTextAtWithMargin(2 + ((XRES) / 8), 24, 311, head->item->description,
                                     head->item->active ? 192 : 255);
            }
            ++line;
        }
        head = head->next;
    }


    for (z = -8; z < 8; ++z) {
        for (x = -13; x < 13; ++x) {
            fill(XRES + 8 + (13 * 4) + (4 * x), 2 + 8 + (8 * 4) + (4 * z), 4, 4,
                 (x == 0 && z == 0) ? 32 : isPositionAllowed(visPos.x + x, visPos.y + z) ? 192 : 64, FALSE);
        }
    }
}
