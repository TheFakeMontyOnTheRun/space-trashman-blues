#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include "FixP.h"
#include "Vec.h"

void initVec2i(struct Vec2i * vec, int8_t x, int8_t y) {
    vec->x = x;
    vec->y = y;
}

void addToVec3(struct Vec3 * to, FixP_t x, FixP_t y, FixP_t z) {

    to->mX = (to->mX + x);
    to->mY = (to->mY + y);
    to->mZ = (to->mZ + z);
}

