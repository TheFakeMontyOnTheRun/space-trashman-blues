#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif
#include "FixP.h"
#include "Vec.h"

void initVec2i(struct Vec2i *__restrict__ vec, int8_t x, int8_t y) {
    vec->x = x;
    vec->y = y;
}

void initVec3(struct Vec3 *__restrict__ vec, FixP_t x, FixP_t y, FixP_t z) {
    vec->mX = x;
    vec->mY = y;
    vec->mZ = z;
}

void addToVec3(struct Vec3 *__restrict__ to, FixP_t x, FixP_t y, FixP_t z) {

    to->mX = (to->mX + x);
    to->mY = (to->mY + y);
    to->mZ = (to->mZ + z);
}
