//
// Created by Daniel Monteiro on 2021-11-01.
//
#include <stdint.h>
#define NULL (0)

#ifndef SMD
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#include "Core.h"
#include "Derelict.h"
#include "Engine3D.h"
#include "HackingMinigame.h"

extern uint8_t accessGrantedToSafe;
extern uint8_t cursorPosition;

char *functionNames[5] = {
        "???",
        "writeB",
        "snprintf",
        "hackLogin",
        "LoadDBReg"
};

uint8_t holdingDisk = 3;
uint8_t pins[3][5];
uint8_t pinTop[3];

void HackingScreen_initStateCallback(void) {

    cursorPosition = 1;
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

void HackingScreen_repaintCallback(void) {
    uint8_t pin;

    drawWindow(1, 1, 40, 15, "Disassembly: CONTROLLER.PRG (stack)");

    writeStr((12 * 0), 11, cursorPosition == 0 ? "[CPU0]" : " CPU0 ", 0, 0);
    writeStr((12 * 1), 11, cursorPosition == 1 ? "[CPU1]" : " CPU1 ", 0, 0);
    writeStr((12 * 2), 11, cursorPosition == 2 ? "[CPU2]" : " CPU2 ", 0, 0);

    writeStr(0, 10, "-------------------------------", 1, 0);


    for (pin = 0; pin < 3; ++pin) {
        uint8_t disk;

        if (pins[pin][4] == 0) {
            accessGrantedToSafe = TRUE;
        }

        for (disk = 0; disk < 5; ++disk) {

            uint8_t diskIndex = pins[pin][disk];

            char *funcName = (disk >= pinTop[pin]) ? NULL : functionNames[diskIndex];

            if (pin != 0) {
                writeStr(10 * (pin), 4 + (4 - disk), "I", 1, 0);
            }

            if (funcName) {
                writeStr(10 * (pin) + (pin == 0 ? 0 : 1), 4 + (4 - disk), funcName, 1, 0);
            }
        }
    }

    writeStr(1, 2, "register pointer:", 0, 15);

    if (holdingDisk != 0xFF) {
        writeStr(19, 2, functionNames[holdingDisk], 128, 15);
    } else {
        writeStr(19, 2, "NULL", 128, 15);
    }
}

int8_t HackingScreen_tickCallback(char cmd) {

    uint8_t pin;

    for (pin = 0; pin < 3; ++pin) {
        if (pins[pin][4] == 0) {
            accessGrantedToSafe = TRUE;
        }
    }

    switch (cmd) {
        case 'q':
        case 'a':
            if (cursorPosition > 0) {
                cursorPosition--;
            }
            break;
        case 'e':
        case 'd':
            if (cursorPosition < 2) {
                cursorPosition++;
            }
            break;
        case 's':
            return -1;
        case 'w':
            if (holdingDisk == 0xFF) {
                if (pinTop[cursorPosition] > 0) {
                    pinTop[cursorPosition]--;
                    holdingDisk = pins[cursorPosition][pinTop[cursorPosition]];
                }
            } else {

                if (pinTop[cursorPosition] == 0 || holdingDisk < pins[cursorPosition][pinTop[cursorPosition] - 1]) {
                    pins[cursorPosition][pinTop[cursorPosition]] = holdingDisk;
                    pinTop[cursorPosition]++;
                    holdingDisk = 0xFF;
                }
            }
            break;

        default:
            break;
    }

    return 0;
}

void runHackingMinigame(void) {
    uint8_t cmd;
    clearScreen();
    HackingScreen_initStateCallback();
    HackingScreen_repaintCallback();
    uint8_t state = 0;

    while (state == 0) {
        cmd = '.';

        while (cmd == '.') {
            cmd = getKey();
        }

        state = HackingScreen_tickCallback(cmd);
        clearScreen();
        HackingScreen_repaintCallback();
    }

    clearScreen();
    HUD_initialPaint();
}
