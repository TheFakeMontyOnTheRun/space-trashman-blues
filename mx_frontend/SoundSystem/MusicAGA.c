#include <exec/types.h>
#include <exec/memory.h>
#include <devices/audio.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "PCMAudio.h"
#include "CPackedFileReader.h"

//Lifted from http://amigadev.elowar.com/read/ADCD_2.1/Devices_Manual_guide/node0161.html

/*
#define MENU_SELECTION_CHANGE_SOUND 0
#define STATE_CHANGE_SOUND 1
#define INFORMATION_ACQUIRED_SOUND 2
#define FAILED_TO_GET_INFORMATION_SOUND 3
#define PLAYER_GOT_DETECTED_SOUND 4
#define PLAYER_FIRING_GUN 5
#define ENEMY_FIRING_GUN 6
#define PLAYER_GET_HURT_SOUND 7
*/
UBYTE           whichannel[4][1] = { {1}, {2}, {4}, {8} };
char* files[] = {"menu_move.raw", "menu_select.raw", "gotclue.raw", "detected2.raw", "bong.raw", "fire.raw", "enemyfire.raw", "detected2.raw" };
struct MsgPort *AudioMP[4];    /* Pointer to a port so the device can reply */
struct IOAudio *audioSink[4];    /* Pointer to the I/O block for I/O commands */
ULONG           device[4];
LONG            frequency = 22050;      /* Frequency of the tone desired   */
LONG            clock     = 3579545;  /* Clock constant, 3546895 for PAL */
BYTE           *waveptr[8];              /* Pointer to the sample bytes     */
LONG            samples[8];        /* Number of sample bytes          */

extern struct GfxBase *GfxBase;

int nextChannel = 0;

void playSound( const int action ) {
	nextChannel = (nextChannel + 1 ) % 4;
	/* Flush the previous sound to avoid overlapping */
	AbortIO((struct IORequest *) audioSink[nextChannel]);
	GetMsg(AudioMP[nextChannel]);

	/* write new sound */
	audioSink[nextChannel]->ioa_Request.io_Message.mn_ReplyPort = AudioMP[nextChannel];
	audioSink[nextChannel]->ioa_Request.io_Command = CMD_WRITE;
	audioSink[nextChannel]->ioa_Request.io_Message.mn_Node.ln_Pri = -70;
	audioSink[nextChannel]->ioa_Request.io_Flags = ADIOF_PERVOL | IOF_QUICK;
	audioSink[nextChannel]->ioa_Data = (BYTE *) waveptr[action];
	audioSink[nextChannel]->ioa_Length = samples[action];
	audioSink[nextChannel]->ioa_Period = clock / (2 * frequency);
	audioSink[nextChannel]->ioa_Volume = 64;
	audioSink[nextChannel]->ioa_Cycles = 1;

	BeginIO((struct IORequest *) audioSink[nextChannel] );
}

void setupOPL2(int unused) {

	if (GfxBase->DisplayFlags & PAL)
		clock = 3546895;        /* PAL clock */
	else
		clock = 3579545;        /* NTSC clock */

	for ( int c = 0; c < 4; ++c ) {
		AudioMP[c] = CreatePort(0,0);

		if (AudioMP[c] == 0) {
			goto killaudio;
		}

		audioSink[c] = (struct IOAudio *)
				AllocMem( sizeof(struct IOAudio),MEMF_PUBLIC | MEMF_CLEAR);

		if (audioSink[c] == 0) {
			goto killaudio;
		}

		audioSink[c]->ioa_Request.io_Message.mn_ReplyPort = AudioMP[c];
		audioSink[c]->ioa_Request.io_Message.mn_Node.ln_Pri = -70;
		audioSink[c]->ioa_Request.io_Command = ADCMD_ALLOCATE;
		audioSink[c]->ioa_Request.io_Flags = ADIOF_NOWAIT;
		audioSink[c]->ioa_AllocKey = 0;
		audioSink[c]->ioa_Data = whichannel[c];
		audioSink[c]->ioa_Length = sizeof(whichannel[c]);

		device[c] = OpenDevice(AUDIONAME, 0L, (struct IORequest *) audioSink[c], 0L);

		if (device[c] != 0) {
			goto killaudio;
		}
	}



	for ( int c = 0; c < 8; ++c ) {

		struct PCMAudio audio;

		uint8_t* pcm = loadBinaryFileFromPath(files[c]);
		size_t size = sizeOfFile(files[c]);
		initAudio(pcm, size, &audio);

		samples[c] = audio.size;

		waveptr[c] = (BYTE *) AllocMem(samples[c], MEMF_CHIP | MEMF_PUBLIC);

		if (waveptr[c] == 0) {
			goto killaudio;
		}

		memcpy(waveptr[c], audio.data, samples[c]);
		free(audio.data);
	}
	return;

	killaudio:
	exit(0);
}

void shutdownSounds(void) {
	for ( int c = 0; c < 8; ++c ) {
		if (waveptr[c] != 0) {
			FreeMem(waveptr[c], samples[c]);
		}
	}

	for ( int c = 0; c < 4; ++c ) {
		if (device[c] == 0) {
			CloseDevice( (struct IORequest *) audioSink[c] );
		}

		if (AudioMP[c] != 0) {
			DeletePort(AudioMP[c]);
		}

		if (audioSink[c] != 0) {
			FreeMem( audioSink[c], sizeof(struct IOAudio) );
		}
	}
}

void stopSounds(void) {
}

void soundTick(void) {
}

void muteSound(void) {}
