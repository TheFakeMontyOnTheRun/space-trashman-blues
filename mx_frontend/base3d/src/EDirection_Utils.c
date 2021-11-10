#include <assert.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "Enums.h"
#include "Enums.h"
#include "FixP.h"
#include "Vec.h"
#include "Common.h"
#include "EDirection_Utils.h"

struct Vec2i mapOffsetForDirection(const enum EDirection aDirection) {

    struct Vec2i toReturn;

    switch (aDirection) {
        case kEast:
            initVec2i(&toReturn, 1, 0);
            break;
        case kWest:
            initVec2i(&toReturn, -1, 0);
            break;
        case kSouth:
            initVec2i(&toReturn, 0, 1);
            break;
        case kNorth:
            initVec2i(&toReturn, 0, -1);
            break;
        default:
            assert (FALSE);
    }

    return toReturn;
}

enum EDirection leftOf(const enum EDirection d) {
    switch (d) {
        case kNorth:
            return kWest;
        case kSouth:
            return kEast;
        case kEast:
            return kNorth;
        case kWest:
            return kSouth;
        default:
            return d;
    }
}

enum EDirection rightOf(const enum EDirection d) {
    switch (d) {
        case kNorth:
            return kEast;
        case kSouth:
            return kWest;
        case kEast:
            return kSouth;
        case kWest:
            return kNorth;
        default:
            assert (FALSE);
    }
}

int oppositeOf(int d) {
    switch (d) {
        case 0:
            return 2;
        case 2:
            return 0;
        case 1:
            return 3;
        case 3:
            return 1;
        case 5:
            return 4;
        case 4:
            return 5;
        default:
            assert (FALSE);
    }
}
