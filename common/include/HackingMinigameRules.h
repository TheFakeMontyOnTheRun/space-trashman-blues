/*
  Created by Daniel Monteiro on 2023-11-04.
*/

#ifndef HACKINGMINIGAMERULES_H
#define HACKINGMINIGAMERULES_H

void initHackingMinigame(void);

uint8_t getHoldingDisk(void);

uint8_t getDisksForPin(uint8_t pin);

uint8_t getPositionForPin(uint8_t pin, uint8_t slot);

uint8_t isHackingMinigameCompleted();

void pickDisk(uint8_t pin);

void dropDisk(uint8_t pin);

#endif /* HACKINGMINIGAMERULES_H */
