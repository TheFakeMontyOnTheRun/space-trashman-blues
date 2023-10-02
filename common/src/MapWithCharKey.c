#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#include <stddef.h>

#ifndef SMD
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#endif

#include "MapWithCharKey.h"

const void *getFromMap(const struct MapWithCharKey * map, const uint8_t key) {
    return map->mMap[key];
}

void setInMap(struct MapWithCharKey * map,
              const uint8_t key,
              const void * value) {
    map->mMap[key] = value;
}

void clearMap(struct MapWithCharKey * map) {
    memFill(map->mMap, 0, sizeof(const void *) * 256);
}

