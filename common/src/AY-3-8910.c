/*
 * AY-3-8910: used on MSX, Amstrad CPC 464 and Atari ST
 * Created by Daniel Monteiro on 11/07/2023.
 * */

#include "AY-3-8910.h"

#include <psg.h>
#include <sound.h>

void initAY38910(void) {
    psg_init();
    psg_channels(chanAll, chanNone); /*  set all channels to tone generation */
    psg_tone(0, psgT(130.8)); // produce a C tone on the first channel
    psg_tone(1, psgT(164.8)); // produce a E tone on the second channel
    psg_tone(2, psgT(195.9)); // produce a G tone on the third channel

    psg_envelope(envUH, psgT(10), chanAll); // set a raising volume envelope on all channels
}

void stopSounds(void) {}

void soundTick(void) {}

void playSound( int ununused ) {}
