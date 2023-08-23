#ifndef VISIBILITY_H
#define VISIBILITY_H

enum EVisibility {
    kInvisible, kVisible
};

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos);

void castVisibility(const enum EDirection from,
                    enum EVisibility *visMap,
                    const struct Vec2i pos,
                    struct Vec2i *distances,
                    const int cleanPrevious,
                    const struct MapWithCharKey *occluderTiles);

#endif
