#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif

#include "Common.h"

void clearVector(struct ItemVector *vector) {
	size_t c;
    vector->used = 0;
    for (c = 0; c < vector->capacity; ++c) {
        if (vector->items[c] != NULL) {
            free(vector->items[c]);
            vector->items[c] = NULL;
        }
    }
}

void initVector(struct ItemVector *vector, size_t capacity) {
    vector->capacity = capacity;
    vector->used = 0;
    vector->items = (void **) (calloc(capacity, sizeof(void *)));
}

int removeFromVector(struct ItemVector *vector, void *item) {
	size_t c, d;
    for (c = 0; c < vector->capacity; ++c) {
        if (vector->items[c] == item) {
            void *replacement = NULL;

            for (d = vector->used - 1; d >= c + 1; --d) {
                replacement = vector->items[d];
                if (replacement != NULL) {
                    vector->items[d] = NULL;
                    break;
                }
            }
            vector->used--;
            vector->items[c] = replacement;

            return 1;
        }
    }

    return 0;
}

int pushVector(struct ItemVector *vector, void *item) {
	size_t c;
    for (c = 0; c < vector->capacity; ++c) {
        if (vector->items[c] == NULL) {
            vector->used++;
            vector->items[c] = item;
            return 1;
        }
    }

    return 0;
}
