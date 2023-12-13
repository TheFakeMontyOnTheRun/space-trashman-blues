/*
 * AY-3-8910: used on MSX, Amstrad CPC 464 and Atari ST
 * Created by Daniel Monteiro on 11/07/2023.
 * */

#include "SN76489.h"

#include <games.h>
#include <psg.h>
#include <sound.h>
#include <stdlib.h>
#include <time.h>
#include "Enums.h"
#include "SoundSystem.h"

uint8_t soundPos;
uint8_t soundNum;
uint8_t soundDuration;

#define SOUND_DURATION 4

const uint8_t sounds[] = {
                0,
                3,
                130,
                195,
                205,
                3,
                170,
                130,
                205,
                2,
                130,
                104,
                0
};

void initSN76489(void) {
#ifdef PSG
    psg_init();
    psg_channels(chanAll, chanNone); // set all channels to tone generation
#endif

    psg_tone(0, 0);
    soundPos = 0;
    soundNum = 0;
    soundDuration = 0;
    psg_volume(0, 10);
}

void stopSounds(void) {
    soundNum = 0;
}

void soundTick(void) {
    if (soundPos < sounds[soundNum]) {
        psg_tone(0, (int)(223878 / (sounds[soundNum + soundPos + 1])));
        psg_envelope(envUH, (int)(223878 / 10), chanAll);

        if (!(soundDuration--)) {
            ++soundPos;
            soundDuration = SOUND_DURATION;
        }
    } else {
        psg_tone(0, 0);
        soundPos = 0;
        soundNum = 0;
    }
}


void playSound(int num) {
    soundPos = 0;
    soundNum = 0;

    while (--num) {
        soundNum += 1 + sounds[soundNum];
    }

    soundDuration = SOUND_DURATION;
}
