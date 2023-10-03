/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

void initStation(void);

uint8_t getPlayerRank(void);

void setPlayerRank(uint8_t newRank);

void useCardWithCardWriter(struct Item *item1, struct Item *item2);

void useBootsWithMagneticCoupling(struct Item *item1, struct Item *item2);

void useCloggedFlush(struct Item *item);

void useRegularFlush(struct Item *item);

void elevatorGoDownCallback(struct Item *item);

void elevatorGoUpCallback(struct Item *item);

void grantAccessToSafe();

uint8_t isAccessToSafeGranted();

#endif
