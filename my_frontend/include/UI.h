/*
 Created by Daniel Monteiro on 14/07/2023.
*/

#ifndef DERELICT8_MENU_H
#define DERELICT8_MENU_H

void titleScreen(void);

void writeStr(uint8_t _x, uint8_t y, const char *text);

void showMessage(const char *message);

void drawMap(void);

void performAction(void);

void drawWindow(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th, const char *title, uint8_t colour);

void drawGraphic(const uint8_t *graphic);

#ifdef SDLW
void flushVirtualFramebuffer(void);
#endif

#endif //DERELICT8_MENU_H
