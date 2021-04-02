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
#include "Common.h"
#include "LoadBitmap.h"
#include "Engine.h"
#include "CRenderer.h"


#include "UI.h"

void drawWindow(const int x, const int y, const int dx, const int dy, const char *__restrict__ title) {

    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 15, FALSE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 4);
}

void drawTextWindow(const int x, const int y, const int dx, const int dy, const char *__restrict__ title,
                    const char *__restrict__ content) {
    drawWindow(x, y, dx, dy, title);
    drawTextAt(x + 1, y + 2, content,

#ifdef AGA5BPP
            7
#else
               0
#endif
            );
}

void drawImageWindow(const int x, const int y, const int dx, const int dy, const char *__restrict__ title,
                     const struct Bitmap *__restrict__ content) {
    fill((x) * 8, (y) * 8, dx * 8, dy * 8, 0, TRUE);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 15, FALSE);
    drawBitmap((x - 1) * 8, (y) * 8, content, TRUE);
    drawRect((x - 1) * 8, (y - 1) * 8, dx * 8, dy * 8, 0);
    fill((x - 1) * 8, (y - 1) * 8, dx * 8, 8, 0, FALSE);
    drawTextAt(x + 1, y, title, 4);
}
