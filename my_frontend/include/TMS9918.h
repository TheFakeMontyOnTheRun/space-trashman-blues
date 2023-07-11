/*
   Created by Daniel Monteiro on 11/07/2023.
*/

#ifndef DERELICT8_TMS9918_H
#define DERELICT8_TMS9918_H

void flush3DBuffer(void);
void initTMS9918(void);
void writeStrWithLimit(uint8_t _x, uint8_t y, char *text, uint8_t limitX);
void realPut(uint8_t x, uint8_t y);
void drawLine(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1);
#endif /* DERELICT8_TMS9918_H */
