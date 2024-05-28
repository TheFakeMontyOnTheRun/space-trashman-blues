/*
 Created by Daniel Monteiro on 15/11/2019.
*/

#ifndef PCMAUDIO_H
#define PCMAUDIO_H
struct PCMAudio {
	size_t size;
	long lengthInMS;
	uint8_t *data;
};

void initAudio(uint8_t *src, size_t size, struct PCMAudio *dst);

#endif /* PCMAUDIO_H */
