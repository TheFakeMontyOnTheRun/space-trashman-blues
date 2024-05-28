#include <stdint.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <sys/farptr.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <bios.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "OPL2.h"
#include "controller.h"
#include "timer.h"
#include "Enums.h"

bool timerSet = false;
int timerStart = 0;
bool enableOPL2 = false;
extern OPL2 opl2;
char SoundBuffer[3][255];
uint32_t melody[256];
uint32_t melody_used = 0;

uint32_t *currentSoundEnd = &melody[0];
uint32_t *currentSoundPosition = &melody[0];

int lastFreq = -1;

short get_lpt_port(int i) {
    return _farpeekw(_dos_ds, 0x0408 + (2 * (i - 1)));
}

short setup(void) {
    cputs("OPT2LPT setup\r\n\r\n");

    char num_ports, port, i;

    num_ports = 0;
    for (i = 1; i < 4; i++) {
        if (get_lpt_port(i)) {
            num_ports++;
            port = i;
        }
    }

    if (num_ports == 0) {
        cputs("Sorry, no printer port found...\r\n");
        exit(1);
    } else if (num_ports == 1) {
        cprintf("Found one printer port: LPT%d\r\n", port);
        return get_lpt_port(port);
    } else {
        cputs("Found multiple printer ports:");
        for (i = 1; i < 4; i++) {
            if (get_lpt_port(i)) {
                cprintf(" LPT%d", i);
            }
        }
        cputs("\r\nWhich one is the OPT2LPT connected to? [");
        for (i = 1; i < 4; i++) {
            if (get_lpt_port(i)) {
                cprintf("%d", i);
            }
        }
        cputs("]? ");
        do {
            port = getch() - '0';
        } while (port < 1 || port > 3 || !get_lpt_port(port));
        cprintf("LPT%d\r\n", port);
        return get_lpt_port(port);
    }
    return 0;
}

void playMusic(int channel, const char *m) {

    if (enableOPL2) {
        if (m == NULL || strlen(m) == 0) {
            music_stop();
            return;
        }

        music_set(channel, m, 5);
        return;
    } else {
        uint32_t frequency = 0;
        int octave = 0;
        melody_used = 0;
        size_t len = strlen(m);

        for (int c = 0; c < len; ++c) {
            char note = m[c];
            switch (note) {
                case '>':
                    octave++;
                    break;
                case '<':
                    octave--;
                    break;
                case 'o':
                    octave = (m[c + 1]) - '0';
                    break;
                case 'a':
                case 'A':
                    frequency = 932;
                    break;
                case 'B':
                case 'b':
                    frequency = 988;
                    break;
                case 'C':
                case 'c':
                    frequency = 1109;
                    break;
                case 'D':
                case 'd':
                    frequency = 1175;
                    break;
                case 'E':
                case 'e':
                    frequency = 1318;
                    break;
                case 'F':
                case 'f':
                    frequency = 1397;
                    break;
                case 'G':
                case 'g':
                    frequency = 1568;
                    break;
                case ';':
                    melody_used = 0;
                default:
                    frequency = 0;
            }

            int shifter = octave;

            if (octave > 0) {
                frequency = frequency << shifter;
            }

            melody[melody_used++] = frequency;
        }

        if (currentSoundPosition != currentSoundEnd
            && currentSoundPosition == &melody[0]) {
            return;
        }

        currentSoundPosition = &melody[0];
        currentSoundEnd = currentSoundPosition + melody_used - 1;
    }
}

void muteSound() {
    playMusic(0, "");
    playMusic(1, "");
    playMusic(2, "");
    playMusic(3, "");
    playMusic(4, "");
    playMusic(5, "");
    playMusic(6, "");
    playMusic(7, "");
}

extern "C" void playSound(const uint8_t action) {
    switch (action) {
        case STATE_CHANGE_SOUND:
            playMusic(action, "t300i12o2b20");
            break;
        case MENU_SELECTION_CHANGE_SOUND:
            playMusic(action, "t300i12o1b20");
            break;


        case MAIN_MENU_THEME: /* as if... */
            playMusic(action, "t300i20o1d20c20");
            break;
    }
}

extern "C" void setupOPL2(int port) {
    if (port == -1) {
        short lpt_base = setup();
        opl2.init(lpt_base, false);
    } else {
        opl2.init(0x0388, true);
    }
    music_setup();
    enableOPL2 = true;
}

extern "C" void soundTick() {
    if (enableOPL2) {
        music_loop();
    } else {
        uint32_t offset = currentSoundPosition - &melody[0] + 1;

        if (currentSoundPosition < currentSoundEnd && melody_used != 0) {

            currentSoundPosition = &melody[0] + offset;

            uint32_t freq = *currentSoundPosition;

            if (freq != lastFreq) {

                sound(freq);
            }

            lastFreq = freq;

        } else {
            lastFreq = -1;
            nosound();
            currentSoundPosition = &melody[0];
            currentSoundEnd = &melody[0];
        }
    }
}

extern "C" void stopSounds() {
    if (enableOPL2) {
        music_shutdown();
    } else {
        nosound();
        currentSoundPosition = &melody[0];
        currentSoundEnd = &melody[0];
    }
}
