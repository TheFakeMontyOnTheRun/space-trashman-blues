#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Core.h"
#include "Engine.h"
#include "CActor.h"
#include "Common.h"
#include "PackedFileReader.h"
#include "LoadBitmap.h"
#include "CRenderer.h"
#include "CTile3DProperties.h"
#include "FixP.h"
#include "MapWithCharKey.h"
#include "VisibilityStrategy.h"
#include "UI.h"

char* functionNames[6] = {
        "???",
        "writeB",
        "snprintf",
        "hackLogin",
        "LoadDBFile",
        "MainRunLoop",
};

uint8_t holdingDisk = 3;
uint8_t pins[3][10];
uint8_t pinTop[3];
extern uint8_t accessGrantedToSafe;
int wasSmoothMovementPreviouslyEnabled;

void HackingScreen_initStateCallback(int32_t tag) {

    cursorPosition = 1;
    currentPresentationState = kAppearing;
    timeUntilNextState = 500;

    memset(&pins[0][0], 0xFF, sizeof(pins));
    
    pins[0][0] = 5;
    pins[0][1] = 4;
    pins[0][2] = 2;
    pins[0][3] = 1;
    pins[0][4] = 0;
    
    pinTop[0] = 5;
    pinTop[1] = 0;
    pinTop[2] = 0;
    holdingDisk = 3;
    wasSmoothMovementPreviouslyEnabled = enableSmoothMovement;
    enableSmoothMovement = FALSE;
}

void HackingScreen_initialPaintCallback(void) {
}

void HackingScreen_repaintCallback(void) {
    uint8_t isSelected;
    int pin;
    
	fill(0, 0, 319, 199, getPaletteEntry(0xFF6cb1a3), 0);

    drawWindow(1, 1, 40, 15, "Disassembly: CONTROLLER.PRG (stack)");
    
    drawTextAt( 6 + (12 * 0), 11, "CPU0", cursorPosition == 0 ? 128 : 0);
    drawTextAt( 6 + (12 * 1), 11, "CPU1", cursorPosition == 1 ? 128 : 0);
    drawTextAt( 6 + (12 * 2), 11, "CPU2", cursorPosition == 2 ? 128 : 0);
    
    for ( pin = 0; pin < 3; ++pin ) {
        int disk;
        int isCursorOnThisPin = cursorPosition == pin;
        
        if (pins[pin][5] == 0 ) {
            accessGrantedToSafe = TRUE;
        }
        
        for (disk = 0; disk < 6; ++disk) {
            
            int diskIndex = pins[pin][disk];
            
            
            char *funcName = (disk >= pinTop[pin]) ? NULL : functionNames[diskIndex];
                
            if (isCursorOnThisPin) {
                isSelected = 128;
            } else if (diskIndex == 3){
                isSelected = 64;
            } else {
                isSelected = 0;
            }
                
            if (accessGrantedToSafe ) {
                isSelected = getPaletteEntry(0xFF00AA00);
            }
                
            drawTextAt( 13 * (pin) + 1, 4 + (5 - disk), "|", isSelected);
                
            if (funcName) {
                drawTextAt( 13 * (pin) + 2, 4 + (5 - disk), funcName, isSelected);
            }
            
            drawTextAt( 13 * (pin) + 1, 10, "-------------", isSelected);
        }
    }
    
    drawTextAt(1,2, "register pointer:", 0);
    
    if (holdingDisk != 0xFF ) {
        drawTextAt(19,2, functionNames[holdingDisk], 128);
    } else {
        drawTextAt(19,2, "NULL", 128);
    }
}

enum EGameMenuState HackingScreen_tickCallback(enum ECommand cmd, long delta) {

    timeUntilNextState -= delta;

    if (timeUntilNextState <= 0) {

        switch (currentPresentationState) {
            case kAppearing:
                timeUntilNextState = 500;
                currentPresentationState = kWaitingForInput;
                break;
            case kWaitingForInput:
                break;
            case kConfirmInputBlink1:
            case kConfirmInputBlink2:
            case kConfirmInputBlink3:
            case kConfirmInputBlink4:
            case kConfirmInputBlink5:
            case kConfirmInputBlink6:
                timeUntilNextState = 250;
                currentPresentationState =
                        (enum EPresentationState) ((int) currentPresentationState + 1);
                break;
            case kFade:
                return nextNavigationSelection;
        }
    }

    if (currentPresentationState == kWaitingForInput) {
        
        int pin;
        
        for ( pin = 0; pin < 3; ++pin ) {
            if (pins[pin][5] == 0 ) {
                accessGrantedToSafe = TRUE;
            }
        }
        
        switch (cmd) {
            case kCommandLeft:
                if (cursorPosition > 0 ) {
                    cursorPosition--;
                }
                turnTarget = turnStep;
                break;
            case kCommandRight:
                if (cursorPosition < 2 ) {
                    cursorPosition++;
                }
                turnTarget = turnStep;
                break;
            case kCommandBack:
            case kCommandDown:
                return kBackToGame;
            case kCommandFire1:
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
    }

    return kResumeCurrentState;
}

void HackingScreen_unloadStateCallback() {
    enableSmoothMovement = wasSmoothMovementPreviouslyEnabled;
}
