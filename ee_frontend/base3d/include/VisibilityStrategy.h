#ifndef VISIBILITY_H
#define VISIBILITY_H

enum EVisibility {
    kInvisible, kVisible
};

int isValid(const struct Vec2i pos);

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos);

void castVisibility(const enum EDirection from,
                    enum EVisibility *visMap,
                    const uint8_t *occluders,
                    const struct Vec2i pos,
                    struct Vec2i *distances,
                    const int cleanPrevious,
                    const struct MapWithCharKey *occluderTiles);

#endif
