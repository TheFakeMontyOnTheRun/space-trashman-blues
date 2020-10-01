#ifndef MAPCHAR_H
#define MAPCHAR_H

struct MapWithCharKey {
	const void *mMap[256];
};

const void *getFromMap(const struct MapWithCharKey * __restrict__ map, const uint8_t key);

void setInMap(struct MapWithCharKey *__restrict__ map,
			  const uint8_t key,
			  const void * __restrict__ value);

void clearMap(struct MapWithCharKey *map);

#endif
