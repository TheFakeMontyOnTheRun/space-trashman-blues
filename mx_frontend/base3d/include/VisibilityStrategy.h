#ifndef VISIBILITY_H
#define VISIBILITY_H

enum EVisibility {
	kInvisible, kVisible
};

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos);

void castVisibility(const enum EDirection from,
					enum EVisibility * __restrict__ visMap,
					const uint8_t * __restrict__ occluders,
					const struct Vec2i pos,
					struct Vec2i * __restrict__ distances,
					const int cleanPrevious,
					const struct MapWithCharKey * __restrict__ occluderTiles);

#endif
