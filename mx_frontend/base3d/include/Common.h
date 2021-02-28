#ifndef COMMON_H
#define COMMON_H

#define MAP_SIZE 64

#define TRUE 1
#define FALSE 0

struct ItemVector {
    void **items;
    size_t capacity;
    size_t used;
};

struct StaticBuffer {
    uint8_t *data;
    size_t size;
};

void initVector(struct ItemVector *vector, size_t capacity);

int removeFromVector(struct ItemVector *vector, void *item);

void clearVector(struct ItemVector *vector);

int pushVector(struct ItemVector *vector, void *item);

int isBigEndian(void);

uint32_t toNativeEndianess(const uint32_t val);

#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )

#endif
