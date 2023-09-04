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

void initSN76489(void) {
#ifdef PSG
    psg_init();
	psg_channels(chanAll, chanNone); // set all channels to tone generation
#endif

    psg_volume(0, 10);
    psg_volume(1, 10);
    psg_volume(2, 10);


    psg_tone(0, psgT(130.8)); // produce a C tone on the first channel
    psg_tone(1, psgT(164.8)); // produce a E tone on the second channel
    psg_tone(2, psgT(195.9)); // produce a G tone on the third channel

    psg_envelope(envUH, psgT(10), chanAll); // set a raising volume envelope on all channels
}
