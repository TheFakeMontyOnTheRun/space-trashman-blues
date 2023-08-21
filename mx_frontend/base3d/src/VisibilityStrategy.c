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


extern enum EVisibility *visMap;
extern struct Vec2i *distances;

struct Vec2i transform(const enum EDirection from, const struct Vec2i currentPos) {

    struct Vec2i toReturn;

    switch (from) {
        case kNorth:
             return currentPos;
            break;
        case kSouth:
            toReturn.x = (MAP_SIZE - currentPos.x - 1);
            toReturn.y = (MAP_SIZE - currentPos.y - 1);
            break;

        case kEast:
            toReturn.x = (MAP_SIZE - currentPos.y - 1);
            toReturn.y = (MAP_SIZE - currentPos.x - 1);
            break;
        case kWest:
            toReturn.x = (currentPos.y);
            toReturn.y = (currentPos.x);
            break;
        default:
            assert (FALSE);
    }

    return toReturn;
}

void castVisibility(const enum EDirection from,
                    const struct Vec2i pos,
                    const int cleanPrevious,
                    const struct MapWithCharKey *__restrict__ occluderTiles) {

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
        memFill (visMap, 0, sizeof(kInvisible) * MAP_SIZE * MAP_SIZE);
    }

    *stackHead = originalPos;
    ++stackHead;

    memFill (distances, -128, sizeof(struct Vec2i) * 2 * MAP_SIZE * MAP_SIZE);
    memFill (&bucketPositions, 0, sizeof(uint8_t) * (MAP_SIZE + MAP_SIZE));

    while (stackHead != stackRoot) {
        struct Vec2i transformed;
        int verticalDistance;
	int horizontalDistance;
        int manhattanDistance;
        int narrowing;

        --stackHead;

        currentPos = *stackHead;

	if (from != kNorth) {
	  transformed = transform(from, currentPos);
	} else {
	  transformed = currentPos;
	}
	
	if (!(0 <= transformed.x && transformed.x < MAP_SIZE && 0 <= transformed.y && transformed.y < MAP_SIZE) ) {
	  continue;
	}

        if (visMap[(transformed.y * MAP_SIZE) + transformed.x] == kVisible) {
            continue;
        }

        visMap[(transformed.y * MAP_SIZE) + transformed.x] = kVisible;

        verticalDistance = (currentPos.y - originalPos.y);
	horizontalDistance = (currentPos.x - originalPos.x);
        manhattanDistance = abs(verticalDistance) + abs(horizontalDistance);

        if (manhattanDistance < (2 * MAP_SIZE)) {
            distances[(manhattanDistance * (MAP_SIZE))
                      + (bucketPositions[manhattanDistance]++)] = transformed;
        }

        if (getFromMap(
                occluderTiles,
                LEVEL_MAP(transformed.x, transformed.y))) {
            continue;
        }

        narrowing = abs(verticalDistance) + 3;

	if (stackHead != stackEnd) {
	  if ((horizontalDistance >= -narrowing) && (horizontalDistance <= 0)) {
            initVec2i(stackHead++, (currentPos.x + leftOffset.x), (currentPos.y + leftOffset.y));
	  }
	  
	  if ((horizontalDistance <= narrowing) && (horizontalDistance >= 0)) {
            initVec2i(stackHead++, (currentPos.x + rightOffset.x), (currentPos.y + rightOffset.y));
	  }
	  
	  if (verticalDistance <= 0) {
            initVec2i(stackHead++, (currentPos.x + northOffset.x), (currentPos.y + northOffset.y));
	  }
	}
    }
}
