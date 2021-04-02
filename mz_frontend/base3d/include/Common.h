#ifndef COMMON_H
#define COMMON_H

#define MAP_SIZE 40

#define TRUE 1
#define FALSE 0

int isBigEndian();

uint32_t toNativeEndianess(const uint32_t val);

#endif
