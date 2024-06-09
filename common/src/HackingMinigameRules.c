/*
  Created by Daniel Monteiro on 2023-11-04.
*/
#ifdef WIN32
#include "Win32Int.h"
#else
#ifndef SMD
#include <stdint.h>
#else
#include <genesis.h>
typedef unsigned long size_t;
#endif
#endif

#include <string.h>

#include "Common.h"
#include "HackingMinigameRules.h"

uint8_t holdingDisk = 3;
uint8_t pins[3][5];
uint8_t pinTop[3];


void initHackingMinigame(void) {
    memset(&pins[0][0], 0xFF, sizeof(pins));
    pins[0][0] = 4;
    pins[0][1] = 2;
    pins[0][2] = 1;
    pins[0][3] = 0;

    pinTop[0] = 4;
    pinTop[1] = 0;
    pinTop[2] = 0;
    holdingDisk = 3;
}

uint8_t getHoldingDisk(void) {
    return holdingDisk;
}

uint8_t getDisksForPin(uint8_t pin) {
    return pinTop[pin];
}

uint8_t getPositionForPin(uint8_t pin, uint8_t slot) {
    return pins[pin][slot];
}

uint8_t isHackingMinigameCompleted(void) {
	uint8_t pin;
    for (pin = 0; pin < 3; ++pin) {
        if (pins[pin][4] == 0) {
            return 1;
        }
    }
    return 0;
}

void pickDisk(uint8_t pin) {
    if (pinTop[pin] > 0) {
        pinTop[pin]--;
        holdingDisk = pins[pin][pinTop[pin]];
    }
}

void dropDisk(uint8_t pin) {
    if (pinTop[pin] == 0 || holdingDisk < pins[pin][pinTop[pin] - 1]) {
        pins[pin][pinTop[pin]] = holdingDisk;
        pinTop[pin]++;
        holdingDisk = 0xFF;
    }
}
