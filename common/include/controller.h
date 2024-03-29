#ifndef CONTROLLER_H
#define CONTROLLER_H

extern void
music_set(int channel, const char *melody, int block);

extern void music_setup();

extern void music_loop();

extern void music_shutdown();

extern "C" {
void setupSounds(int port);
void shutdownSounds();
void music_stop();
}

#endif
