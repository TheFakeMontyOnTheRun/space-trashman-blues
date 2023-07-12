/*
   Created by Daniel Monteiro on 11/07/2023.
*/

#include <psg.h>
#include <sound.h>

void initYM2413(void) {
    psg_init();
    psg_channels(chanAll, chanNone); /*  set all channels to tone generation */
    psg_volume(0, 10);
    psg_volume(1, 10);
    psg_volume(2, 10);

    psg_tone(0, psgT(262));

    psg_tone(0, 0);
}
