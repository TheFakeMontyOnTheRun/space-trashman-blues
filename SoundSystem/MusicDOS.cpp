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
		int position = 0;
		size_t len = strlen(m);

		for (int c = 0; c < len; ++c) {
			char note = m[c];
			++position;
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
					frequency = 28;
					break;
				case 'B':
				case 'b':
					frequency = 30;
					break;
				case 'C':
				case 'c':
					frequency = 16;
					break;
				case 'D':
				case 'd':
					frequency = 18;
					break;
				case 'E':
				case 'e':
					frequency = 20;
					break;
				case 'F':
				case 'f':
					frequency = 22;
					break;
				case 'G':
				case 'g':
					frequency = 24;
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

extern "C" void playSound( const int action ) {
	switch (action) {
		case STATE_CHANGE_SOUND:
			playMusic(action, "t300i12o2b20");
			break;
		case PLAYER_GOT_DETECTED_SOUND:
			playMusic(action,"t100i21o1b20i20o1b20i15o1b20i20o1b20i20o1b20i20o1b20i10o1b20i20o1b20");
			break;
		case ENEMY_FIRING_GUN:
			playMusic(action,"t300i12o2b20");
			break;
		case MENU_SELECTION_CHANGE_SOUND:
			playMusic(action,"t300i12o1b20");
			break;

		case PLAYER_FIRING_GUN:
			playMusic(action,"t300i12o2b20");
			break;
		case INFORMATION_ACQUIRED_SOUND:
			playMusic(action,"t200i20o3fdi20o3fdi20o3fdi20o3fdi20o3fdi20o3fdi20o3fdi20o3fdi20o3fd");
			break;
		case FAILED_TO_GET_INFORMATION_SOUND:
			playMusic(action,"t300i20o1b20");
			break;

		case PLAYER_GET_HURT_SOUND:
			playMusic(action,"t300i20o1d20c20");
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

void stopSounds() {
	if (enableOPL2) {
		music_shutdown();
	} else {
		nosound();
		currentSoundPosition = &melody[0];
		currentSoundEnd = &melody[0];
	}
}
