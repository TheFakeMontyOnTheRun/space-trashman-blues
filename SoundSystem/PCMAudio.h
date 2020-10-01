/*
 Created by Daniel Monteiro on 15/11/2019.
*/

#ifndef THE_MISTRAL_REPORT_PCMAUDIO_H
#define THE_MISTRAL_REPORT_PCMAUDIO_H
struct PCMAudio {
	size_t size;
	long lengthInMS;
	uint8_t* data;
};

void initAudio( uint8_t* src, size_t size, struct PCMAudio* dst );
#endif /* THE_MISTRAL_REPORT_PCMAUDIO_H */
