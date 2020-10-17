/*
 Created by Daniel Monteiro on 15/11/2019.
*/
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "PCMAudio.h"


void initAudio( uint8_t* src, size_t size, struct PCMAudio* dst ) {
	dst->size = size;
	dst->data = src;
	dst->lengthInMS = ((size * 1000) / 22050);
}