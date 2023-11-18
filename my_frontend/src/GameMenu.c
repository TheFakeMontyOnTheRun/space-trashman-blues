/*
   Created by monty on 01-10-2023.
*/
#include <stdlib.h>
#include <stdint.h>
#include "Enums.h"
#include "UI.h"
#include "Renderer.h"

void GameMenu_initStateCallback(enum EGameMenuState tag) {}

void GameMenu_initialPaintCallback(void) {}

void GameMenu_repaintCallback(void) {}

enum EGameMenuState GameMenu_tickCallback(enum ECommand cmd, long data) {
    return kMainMenu;
}

void GameMenu_unloadStateCallback(enum EGameMenuState newState) {}

