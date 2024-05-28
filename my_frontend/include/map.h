#ifndef MAP_H
#define MAP_H

#define BASE_CEILING_HEIGHT 7

/* ceiling is at height 12, floor at height 0. Camera is at height 2 */

extern const struct CellPattern patterns[96];

extern int8_t map[32][32];

#endif
