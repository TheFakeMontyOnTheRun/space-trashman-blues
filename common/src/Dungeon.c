#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef WIN32
#include "Win32Int.h"
#else

#include <stdint.h>
#include <unistd.h>

#endif

#include "Common.h"
#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "Dungeon.h"
#include "EDirection_Utils.h"
#include "Core.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Parser.h"
#include "CActor.h"
/* 
   This include must be here just to satisfy the .h - your IDE might trick you into thinking this is not needed. 
   And it's not, but ISO requires. 
*/
#include "Mesh.h"
#include "Renderer.h"


const char *focusItemName = NULL;
struct GameSnapshot gameSnapshot;
uint8_t *map;
uint8_t *itemsInMap;
uint8_t *collisionMap;
int enteredThru = 0;
struct CActor playerCrawler;
int currentSelectedItem = 0;

uint8_t isPositionAllowed(int8_t x, int8_t y) {

    return (0 <= x) && (x < MAP_SIZE) && (0 <= y) && (y < MAP_SIZE)
           && collisionMap[LEVEL_MAP(x, y)] != '1';
}

struct GameSnapshot dungeonTick(const enum ECommand command) {
    int currentPlayerRoom;
    int cell;
    struct WorldPosition worldPos;
    struct ObjectNode *head;

    int oldTurn = gameSnapshot.turn;
    struct WorldPosition oldPosition = *getPlayerPosition();
    currentPlayerRoom = getPlayerRoom();

    {
        switch (command) {
            case kCommandRight:
                playerCrawler.rotation = rightOf(playerCrawler.rotation);
                turnRight();
#ifdef PAGE_FLIP_ANIMATION
                turnStep = PAGE_FLIP_TARGET;
                turnTarget = 0;
#endif
                break;

            case kCommandLeft:
                playerCrawler.rotation = leftOf(playerCrawler.rotation);
                turnLeft();
#ifdef PAGE_FLIP_ANIMATION
                turnStep = 0;
                turnTarget = PAGE_FLIP_TARGET;
#endif
                break;
            case kCommandUp: {
                struct Vec2i offset = mapOffsetForDirection(
                        playerCrawler.rotation);
                gameSnapshot.turn++;

                playerCrawler.position.x += offset.x;
                playerCrawler.position.y += offset.y;


                if (collisionMap[LEVEL_MAP(playerCrawler.position.x, playerCrawler.position.y)] == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(0);
                    if (enableSmoothMovement) {
                        zCameraOffset = intToFix(2);
                    }
                }
            }
                break;
            case kCommandDown: {
                struct Vec2i offset = mapOffsetForDirection(
                        playerCrawler.rotation);
                gameSnapshot.turn++;
                playerCrawler.position.x -= offset.x;
                playerCrawler.position.y -= offset.y;

                if (collisionMap[LEVEL_MAP(playerCrawler.position.x, playerCrawler.position.y)] == '1') {
                    playerCrawler.position.x += offset.x;
                    playerCrawler.position.y += offset.y;
                } else {
                    walkBy(2);
                    if (enableSmoothMovement) {
                        zCameraOffset = -intToFix(2);
                    }
                }
            }
                break;

            case kCommandFire1: {
                int index = 0;
                struct ObjectNode *head1 = getRoom(getPlayerRoom())->itemsPresent->next;
                struct Item *item1 = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                struct ObjectNode *head2 = getPlayerItems();
                struct Item *item2 = NULL;
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;

                needsToRedrawHUD = TRUE;

                while (head2 != NULL && (index < currentSelectedItem)) {
                    ++index;
                    head2 = head2->next;
                }

                if (head2 != NULL) {
                    item2 = getItem(head2->item);
                }

                while (head1 != NULL && item1 == NULL) {
                    if (offseted.x == (getItem(head1->item)->position.x) &&
                        offseted.y == (getItem(head1->item)->position.y)) {
                        item1 = getItem(head1->item);
                    }
                    head1 = head1->next;
                }

                if (item2 != NULL) {
                    if (item1 != NULL) {
                        char buffer[255];
                        char *operator1;
                        char *operand1;

                        sprintf(&buffer[0], "use-with %s %s", item2->name, item1->name);
                        operator1 = strtok(&buffer[0], "\n ");
                        operand1 = strtok(NULL, "\n ");
                        parseCommand(operator1, operand1);
                    } else {
                        parseCommand("use", item2->name);
                    }
                }
                computeLightning();
                gameSnapshot.turn++;
            }
                break;
            case kCommandFire2: {
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                head = getRoom(getPlayerRoom())->itemsPresent->next;
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;

                needsToRedrawHUD = TRUE;

                while (head != NULL && item == NULL) {
                    if (offseted.x == (getItem(head->item)->position.x) &&
                        offseted.y == (getItem(head->item)->position.y)) {
                        item = getItem(head->item);
                    }
                    head = head->next;
                }

                if (item != NULL) {

                    if (item == getItemNamed("computer-terminal")) {
                        enterState(kHackingGame);
                    } else {
                        parseCommand(item->pickable ? "pick" : "use", item->name);
                        if (item->pickable) {
                            setItem(offseted.x, offseted.y, 0xFF);
                        }
                    }
                } else {
                    int index = 0;
                    head = getPlayerItems();
                    item = NULL;

                    while (head != NULL && (index < currentSelectedItem)) {
                        ++index;
                        head = head->next;
                    }

                    if (head != NULL) {
                        item = getItem(head->item);
                    }

                    if (item != NULL && item->pickable) {
                        parseCommand("drop", item->name);
                        item->position.x = offseted.x;
                        item->position.y = offseted.y;
                        setItem(item->position.x, item->position.y, item->index);
                        currentSelectedItem = 0;
                    }
                }
                computeLightning();
                gameSnapshot.turn++;
            }
                break;
            case kCommandFire3: {
                struct ObjectNode *playerItems = getPlayerItems();
                int index = 0;

                needsToRedrawHUD = TRUE;

                ++currentSelectedItem;

                while (playerItems != NULL) {
                    ++index;
                    playerItems = playerItems->next;
                }

                if (currentSelectedItem >= index) {
                    currentSelectedItem = 0;
                }
            }
                break;
            case kCommandNone:
                break;
            case kCommandBack:
                break;
            case kCommandStrafeLeft: {
                struct Vec2i offset =
                        mapOffsetForDirection(leftOf(playerCrawler.rotation));
                gameSnapshot.turn++;

                playerCrawler.position.x += offset.x;
                playerCrawler.position.y += offset.y;

                if (collisionMap[LEVEL_MAP(playerCrawler.position.x, playerCrawler.position.y)] == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(3);
                    if (enableSmoothMovement) {
                        xCameraOffset = -intToFix(2);
                    }
                }
            }
                break;
            case kCommandStrafeRight: {
                struct Vec2i offset =
                        mapOffsetForDirection(rightOf(playerCrawler.rotation));
                gameSnapshot.turn++;

                playerCrawler.position.x += offset.x;
                playerCrawler.position.y += offset.y;

                if (collisionMap[LEVEL_MAP(playerCrawler.position.x, playerCrawler.position.y)] == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(1);
                    if (enableSmoothMovement) {
                        xCameraOffset = intToFix(2);
                    }
                }
            }
                break;
            default:
                break;
        }
    }

    worldPos = *getPlayerPosition();
    playerCrawler.position.x = worldPos.x;
    playerCrawler.position.y = worldPos.y;
    playerCrawler.rotation = getPlayerDirection();

    gameSnapshot.camera_x = playerCrawler.position.x;
    gameSnapshot.camera_z = playerCrawler.position.y;
    gameSnapshot.camera_rotation = playerCrawler.rotation;

    if (oldTurn != gameSnapshot.turn) {

        switch (getGameStatus()) {
            case kBadVictory:
                gameSnapshot.should_continue = kCrawlerGameFinished;
                enterState(kBadVictoryEpilogue);
                return gameSnapshot;

            case kBadGameOver:
                gameSnapshot.should_continue = kCrawlerGameFinished;
                enterState(kBadGameOverEpilogue);
                return gameSnapshot;

            case kGoodVictory:
                gameSnapshot.should_continue = kCrawlerGameFinished;
                enterState(kGoodVictoryEpilogue);
                return gameSnapshot;

            case kGoodGameOver:
                gameSnapshot.should_continue = kCrawlerGameFinished;
                enterState(kGoodGameOverEpilogue);
                return gameSnapshot;
            default:
            case kNormalGameplay:
                break;
        }


        {
            struct ObjectNode *roomItems = getRoom(getPlayerRoom())->itemsPresent->next;
            struct Item *item = NULL;
            struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
            focusItemName = NULL;
            offseted.x += playerCrawler.position.x;
            offseted.y += playerCrawler.position.y;

            while (roomItems != NULL && item == NULL) {
                struct Item *itemCandidate = getItem(roomItems->item);
                if (offseted.x == (itemCandidate->position.x) && offseted.y == (itemCandidate->position.y)) {
                    item = itemCandidate;
                }
                roomItems = roomItems->next;
            }

            if (item != NULL) {
                focusItemName = item->name;
            }
        }

        /* for the elevators */
        if (currentPlayerRoom != getPlayerRoom()) {
            enable3DRendering = FALSE;
            enteredThru = 0;
            setPlayerDirection((enum EDirection)enteredThru);
            initRoom(getPlayerRoom());

            thisMissionName = getRoomDescription();
            thisMissionNameLen = strlen(getRoomDescription());

            setPlayerPosition(&oldPosition);

            return gameSnapshot;
        }

        cell = LEVEL_MAP(playerCrawler.position.x, playerCrawler.position.y);

        if ('0' <= cell && cell <= '3') {

            moveBy(cell - '0');

            if (currentPlayerRoom != getPlayerRoom()) {
                int c = 0;
                int room = getPlayerRoom();
                struct Room *roomPtr = getRoom(room);

                for (c = 0; c < 6; ++c) {
                    if (roomPtr->connections[c] == currentPlayerRoom) {
                        enteredThru = oppositeOf(c);
                    }
                }

                enable3DRendering = FALSE;
                setPlayerDirection((enum EDirection)enteredThru);
                zCameraOffset = intToFix(4);
                currentPresentationState = kRoomTransitioning;
                initRoom(room);
                thisMissionName = getRoomDescription();
                thisMissionNameLen = strlen(thisMissionName);

            } else { /* Something prevented the player from moving - maybe not enough clearance */
                setPlayerPosition(&oldPosition);
                playerCrawler.position.x = oldPosition.x;
                playerCrawler.position.y = oldPosition.y;
                gameSnapshot.camera_x = playerCrawler.position.x;
                gameSnapshot.camera_z = playerCrawler.position.y;
            }

            return gameSnapshot;
        }

        head = getRoom(getPlayerRoom())->itemsPresent->next;

        while (head != NULL) {
            struct Item *itemPtr = getItem(head->item);
            setItem(itemPtr->position.x, itemPtr->position.y, itemPtr->index);
            head = head->next;
        }
    }

    return gameSnapshot;
}

void dungeon_loadMap(const uint8_t *__restrict__ mapData,
                     const char *__restrict__ collisions,
                     const int mapIndex) {
    int8_t x, y;
    const uint8_t *ptr = mapData;
    struct WorldPosition worldPos;
    struct ObjectNode *head;

    gameSnapshot.should_continue = kCrawlerGameInProgress;
    gameSnapshot.camera_rotation = 0;
    playerCrawler.rotation = kNorth;
    memCopyToFrom(collisionMap, (void *) collisions, 256);

    for (y = 0; y < MAP_SIZE; ++y) {
        for (x = 0; x < MAP_SIZE; ++x) {
            char current = *ptr;
            LEVEL_MAP(x, y) = current;
            setItem(x, y, 0xFF);

            if ((current == 's' && enteredThru == 0) ||
                (current == 'w' && enteredThru == 1) ||
                (current == 'n' && enteredThru == 2) ||
                (current == 'e' && enteredThru == 3)
                    ) {
                worldPos.x = x;
                worldPos.y = y;
                enteredThru = -1;
            }

            ++ptr;
        }
        ++ptr;
    }

    x = worldPos.x;
    y = worldPos.y;
    setPlayerPosition(&worldPos);
    playerCrawler.position.x = x;
    playerCrawler.position.y = y;

    head = getRoom(getPlayerRoom())->itemsPresent->next;

    while (head != NULL) {
        struct Item *item = getItem(head->item);
        setItem(item->position.x, item->position.y, item->index);
        head = head->next;
    }

    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;
}
