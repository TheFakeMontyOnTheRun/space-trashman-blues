/*
   Created by monty on 01-10-2023.
*/
#include "Common.h"
#include "Enums.h"

void GameMenu_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
}

void GameMenu_repaintCallback(void) {}

enum EGameMenuState GameMenu_tickCallback(enum ECommand cmd, long data) {
    (void)cmd;
    (void)data;
    return kMainMenu;
}

void GameMenu_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}

