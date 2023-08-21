#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Enums.h"
#include "Common.h"
#include "FixP.h"
#include "Vec.h"
#include "MapWithCharKey.h"
#include "VisibilityStrategy.h"
#include "CActor.h"
#include "Common.h"
#include "LoadBitmap.h"
#include "CTile3DProperties.h"
#include "EDirection_Utils.h"
#include "CRenderer.h"
#include "Core.h"
#include "Engine.h"
#include "FixP.h"
#include "MapWithCharKey.h"
#include "Vec.h"

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos) {

    struct Vec2i toReturn;

    switch (from) {
        case kNorth:
            toReturn = currentPos;
            break;
        case kSouth:
            initVec2i(&toReturn, (int8_t) (MAP_SIZE - currentPos.x - 1),
                      (int8_t) (MAP_SIZE - currentPos.y - 1));
            break;

        case kEast:
            initVec2i(&toReturn, (int8_t) (MAP_SIZE - currentPos.y - 1),
                      (int8_t) (MAP_SIZE - currentPos.x - 1));
            break;
        case kWest:
            initVec2i(&toReturn, (int8_t) (currentPos.y),
                      (int8_t) (currentPos.x));
            break;
        default:
            assert (FALSE);
            break;
    }

    return toReturn;
}

void castVisibility(const enum EDirection from,
                    enum EVisibility *visMap,
                    const uint8_t *occluders,
                    const struct Vec2i pos,
                    struct Vec2i *distances,
                    const int cleanPrevious,
                    const struct MapWithCharKey *occluderTiles) {

    const struct Vec2i originalPos = transform(from, pos);
    struct Vec2i positions[MAP_SIZE + MAP_SIZE];
    struct Vec2i currentPos;

    /* The -1 is due to the fact I will add a new element. */
    struct Vec2i *stackHead = &positions[0];
    struct Vec2i *stackEnd = stackHead + (MAP_SIZE + MAP_SIZE);
    struct Vec2i *stackRoot = stackHead;

    struct Vec2i rightOffset = mapOffsetForDirection(kEast);
    struct Vec2i leftOffset = mapOffsetForDirection(kWest);
    struct Vec2i northOffset = mapOffsetForDirection(kNorth);
    uint8_t bucketPositions[MAP_SIZE + MAP_SIZE];

    if (cleanPrevious) {
        memFill(visMap, 0, sizeof(kInvisible) * MAP_SIZE * MAP_SIZE);
    }

    *stackHead = originalPos;
    ++stackHead;

    memFill(distances, -128, sizeof(struct Vec2i) * 2 * MAP_SIZE * MAP_SIZE);
    memFill(&bucketPositions, 0, sizeof(uint8_t) * (MAP_SIZE + MAP_SIZE));

    while (stackHead != stackRoot) {
        struct Vec2i transformed;
        int verticalDistance;
        int manhattanDistance;
        int narrowing;

        --stackHead;

        currentPos = *stackHead;

        transformed = transform(from, currentPos);

        if (!(0 <= transformed.x && transformed.x < MAP_SIZE && 0 <= transformed.y && transformed.y < MAP_SIZE)) {
            continue;
        }

        if (visMap[(transformed.y * MAP_SIZE) + transformed.x] == kVisible) {
            continue;
        }

        visMap[(transformed.y * MAP_SIZE) + transformed.x] = kVisible;

        verticalDistance = (currentPos.y - originalPos.y);

        manhattanDistance =
                abs(verticalDistance) + abs(currentPos.x - originalPos.x);

        if (manhattanDistance < (2 * MAP_SIZE)) {
            distances[(manhattanDistance * (MAP_SIZE))
                      + (bucketPositions[manhattanDistance]++)] = transformed;
        }

        if (getFromMap(
                occluderTiles,
                occluders[(transformed.y * MAP_SIZE) + transformed.x])) {
            continue;
        }

        narrowing = abs(verticalDistance) + 6;

        if (((currentPos.x - originalPos.x) >= -narrowing)
            && (currentPos.x - originalPos.x) <= 0
            && (stackHead != stackEnd)) {
            initVec2i(stackHead++, (int8_t) (currentPos.x + leftOffset.x),
                      (int8_t) (currentPos.y + leftOffset.y));
        }

        if (((currentPos.x - originalPos.x) <= narrowing)
            && (currentPos.x - originalPos.x) >= 0
            && (stackHead != stackEnd)) {
            initVec2i(stackHead++, (int8_t) (currentPos.x + rightOffset.x),
                      (int8_t) (currentPos.y + rightOffset.y));
        }

        if (verticalDistance <= 0 && (stackHead != stackEnd)) {
            initVec2i(stackHead++, (int8_t) (currentPos.x + northOffset.x),
                      (int8_t) (currentPos.y + northOffset.y));
        }
    }
}
