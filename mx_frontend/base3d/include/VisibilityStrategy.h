#ifndef VISIBILITY_H
#define VISIBILITY_H

enum EVisibility {
	kInvisible, kVisible
};

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos);

void castVisibility(const enum EDirection from,
					const struct Vec2i pos,
					const int cleanPrevious,
					const struct MapWithCharKey * __restrict__ occluderTiles);

#endif
