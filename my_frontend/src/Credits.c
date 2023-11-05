/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"

void CreditsScreen_initStateCallback(int32_t tag) {}

void CreditsScreen_initialPaintCallback(void) {}

void CreditsScreen_repaintCallback(void) {}

enum EGameMenuState CreditsScreen_tickCallback(enum ECommand cmd, long data) {
    return kMainMenu;
}

void CreditsScreen_unloadStateCallback(int32_t newState) {}

