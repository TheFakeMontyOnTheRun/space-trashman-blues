#ifndef UTILS_H
#define UTILS_H

enum EDirection leftOf(const enum EDirection d);

enum EDirection rightOf(const enum EDirection d);

struct Vec2i mapOffsetForDirection(const enum EDirection direction);

uint8_t oppositeOf(uint8_t d);

#endif
