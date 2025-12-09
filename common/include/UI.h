/*
* Created by Daniel Monteiro on 2019-08-02.
*/

#ifndef UI_H
#define UI_H

#include "Common.h"

struct Bitmap; /* temporary - soon this will be refactored out */

void drawMenuBackground(void);

void
drawWindowWithOptions(const int x,
                      const int y,
                      const unsigned int dx,
                      const unsigned int dy,
                      const char *title,
                      const char **options,
                      uint8_t optionsCount,
                      uint8_t selectedOption);

void drawWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title);

void drawTextWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                    const char *content);

void drawImageWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                     struct Bitmap *content);

int drawAppearingWindow(const int x, const int y, const unsigned int dx, const unsigned int dy, const char *title,
                        long remainingTime);

void updateMap(void);

enum EGameMenuState handleCursor(const enum EGameMenuState* options,
				 uint8_t optionsCount,
				 const enum ECommand cmd,
				 enum EGameMenuState backState);

void drawGraphic(uint16_t x, uint8_t  y, uint16_t dx, uint8_t dy, const uint8_t DATA_ADDRESS_SPACE *shapes);

extern int8_t cursorPosition;

void clearScreen(void);

void drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t colour);

#endif /*UI_H*/
