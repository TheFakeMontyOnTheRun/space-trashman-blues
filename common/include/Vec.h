#ifndef VEC_H
#define VEC_H

struct Vec3 {
	FixP_t mX;
	FixP_t mY;
	FixP_t mZ;
};

struct Vec2 {
	FixP_t mX;
	FixP_t mY;
};

struct Vec2i {
	int8_t x;
	int8_t y;
};

void initVec2i(struct Vec2i *  vec, int8_t x, int8_t y);

void addToVec3(struct Vec3 *  to, FixP_t x, FixP_t y, FixP_t z);

#endif
