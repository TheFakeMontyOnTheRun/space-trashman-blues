/*
Created by Daniel Monteiro on 2019-07-26.
*/

#ifndef DERELICT_DERELICT_H
#define DERELICT_DERELICT_H

/**
 * TODO: refactor this to reflect the new story
 */
void initStation(void);

/**
 *
 * @return current player rank
 */
uint8_t getPlayerRank(void);

/**
 * force a new rank for player character
 * range: 0..4
 * TODO: create enum for ranks
 * @param newRank to set for player
 */
void setPlayerRank(uint8_t newRank);

/**
 *
 * @param item1
 * @param item2
 */
void useCardWithCardWriter(struct Item *item1, struct Item *item2);

/**
 *
 * @param item1
 * @param item2
 */
void useBootsWithMagneticCoupling(struct Item *item1, struct Item *item2);

/**
 *
 * @param item
 */
void useCloggedFlush(struct Item *item);

/**
 *
 * @param item
 */
void useRegularFlush(struct Item *item);

/**
 *
 * @param item
 */
void elevatorGoDownCallback(struct Item *item);

/**
 *
 * @param item
 */
void elevatorGoUpCallback(struct Item *item);

/**
 *
 */
void grantAccessToSafe();

/**
 *
 * @return
 */
uint8_t isAccessToSafeGranted();

#endif
