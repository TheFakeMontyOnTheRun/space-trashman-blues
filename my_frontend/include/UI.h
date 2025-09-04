/*
 Created by Daniel Monteiro on 14/07/2023.
*/

#ifndef DERELICT8_MENU_H
#define DERELICT8_MENU_H

#include "Common.h"

void drawTextAt(uint8_t _x, uint8_t y, const char *text, uint8_t colour);

void showMessage(const char *message);

void drawMap(void);

void performAction(void);

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title);

void
drawWindowWithOptions(const uint8_t x,
                      const uint8_t y,
                      const uint8_t dx,
                      const uint8_t dy,
                      const char *title,
                      const char **options,
                      const uint8_t optionsCount,
                      const uint8_t selectedOption);

void drawGraphic(uint16_t x, uint8_t  y, uint16_t dx, uint8_t dy, const uint8_t DATA_ADDRESS_SPACE *graphic);

void drawTextWindow(const uint8_t x, const uint8_t y, const uint8_t dx, const uint8_t dy, const char *title,
                    const char *content);

enum EGameMenuState handleCursor(const enum EGameMenuState* options, uint8_t optionsCount, const enum ECommand cmd, enum EGameMenuState backState);

void clearScreen(void);

#define XRES_TEXT (XRES_FRAMEBUFFER / 8)
#define YRES_TEXT (YRES_FRAMEBUFFER / 8)

extern uint8_t waitForKey;

extern int8_t cursorPosition;

extern uint8_t redrawMap;

extern uint8_t needsToRedrawHUD;

#endif //DERELICT8_MENU_H
