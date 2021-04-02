/*
 Created by Daniel Monteiro on 03/10/2019.
*/
#include <stdint.h>
#include <unistd.h>

#include "Common.h"
#include "Globals.h"
#include "Enums.h"
#include "Engine.h"

struct Bitmap *currentBackgroundBitmap = NULL;
int32_t currentGameMenuState = -1;
int32_t menuStateToReturn = -1;
uint8_t cursorPosition = 0;
int32_t nextNavigationSelection = -1;
long timeUntilNextState = MENU_ITEM_TIME_TO_BECOME_ACTIVE_MS;
enum EPresentationState currentPresentationState;
size_t biggestOption;
const char *mainText = NULL;
int isRunning = TRUE;

int isBigEndian() {
    union {
        uint32_t i;
        char c[4];
    } e = {0x01000000};

    return e.c[0];
}

uint32_t toNativeEndianess(const uint32_t val) {
    uint32_t val2 = val;

    if (isBigEndian()) {
        uint32_t b0, b1, b2, b3;

        b0 = (val & 0x000000ff) << 24u;
        b1 = (val & 0x0000ff00) << 8u;
        b2 = (val & 0x00ff0000) >> 8u;
        b3 = (val & 0xff000000) >> 24u;

        val2 = b0 | b1 | b2 | b3;
    }

    return val2;
}

