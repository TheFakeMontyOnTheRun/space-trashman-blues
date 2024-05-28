/*
   Created by monty on 01-10-2023.
*/

#include "Enums.h"

void GameMenu_initStateCallback(enum EGameMenuState tag) {
    (void)tag;
}

void GameMenu_repaintCallback(void) {}

enum EGameMenuState GameMenu_tickCallback(enum ECommand cmd, void* data) {
    (void)cmd;
    (void)data;
    /* This never gets executed, so not a problem for not having the return value */
}

void GameMenu_unloadStateCallback(enum EGameMenuState newState) {
    (void)newState;
}

