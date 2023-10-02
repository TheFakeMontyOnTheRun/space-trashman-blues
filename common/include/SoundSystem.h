/*
 Created by Daniel Monteiro on 18/11/2019.
*/

#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

void playSound(const int action);

void soundTick(void);

void setupOPL2(int port);

void stopSounds(void);

extern enum ESoundDriver soundDriver;

#endif /* SOUNDSYSTEM_H */
