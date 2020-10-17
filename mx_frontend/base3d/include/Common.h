#ifndef COMMON_H
#define COMMON_H

#define MAP_SIZE 64

#define TRUE 1
#define FALSE 0

int isBigEndian(void);

uint32_t toNativeEndianess(const uint32_t val);

#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )

#endif
