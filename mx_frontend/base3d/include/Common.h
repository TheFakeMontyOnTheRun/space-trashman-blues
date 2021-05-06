#ifndef COMMON_H
#define COMMON_H

#define MAP_SIZE 32

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

struct ItemVector {
    void **items;
    size_t capacity;
    size_t used;
};

struct StaticBuffer {
    uint8_t *data;
    size_t size;
};

#define __restrict__ /*__restrict__*/ 

void initVector(struct ItemVector *vector, size_t capacity);

int removeFromVector(struct ItemVector *vector, void *item);

void clearVector(struct ItemVector *vector);

int pushVector(struct ItemVector *vector, void *item);

int isBigEndian(void);

void initFileReader(const char * __restrict__ dataFilePath);

uint32_t toNativeEndianess(const uint32_t val);

#define min(v1, v2) (( (v1) < (v2) ) ? (v1) : (v2) )
#define max(v1, v2) (( (v1) > (v2) ) ? (v1) : (v2) )

#endif
