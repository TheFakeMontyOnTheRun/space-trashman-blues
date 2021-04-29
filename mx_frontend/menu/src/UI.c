/*
* Created by Daniel Monteiro on 2019-08-02.
*/
#include <stdint.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

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
