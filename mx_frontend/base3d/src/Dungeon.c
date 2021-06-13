#include <stdio.h>

#include <stddef.h>
#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#include <unistd.h>
#endif#include <string.h>
#include <stdlib.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "Dungeon.h"
#include "Common.h"
#include "EDirection_Utils.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Core.h"
#include "Derelict.h"
#include "Parser.h"
#include "EDirection_Utils.h"

/* This include must be here just to satisfy the .h - your IDE might trick you into thinking this is not needed. And it's not, but ISO requires. */
#include "CActor.h"

#include "CRenderer.h"

struct GameSnapshot gameSnapshot;
uint8_t map[MAP_SIZE][MAP_SIZE];
uint8_t collisionMap[256];
int enteredThru = 0;
extern char *focusItemName;

extern int currentSelectedItem;

extern int shouldContinue;
extern char *thisMissionName;
extern int16_t thisMissionNameLen;

struct CrawlerAgent {
    struct Vec2i position;
    enum EDirection rotation;
    char symbol;
};

struct CrawlerAgent playerCrawler;

int isPositionAllowed(int x, int y) {

    return (0 <= x) && (x < MAP_SIZE) && (0 <= y) && (y < MAP_SIZE)
           && collisionMap[map[y][x]] != '1';
}

struct GameSnapshot dungeon_tick(const enum ECommand command) {
	int currentPlayerRoom;
	int cell;
	struct WorldPosition worldPos;
	struct ObjectNode *head;
    int oldTurn = gameSnapshot.turn;
    struct WorldPosition oldPosition = *getPlayerPosition();
    setActor(playerCrawler.position.x, playerCrawler.position.y, 0xFF);
    currentPlayerRoom = getPlayerRoom();

    {
        switch (command) {
            case kCommandRight:
                playerCrawler.rotation = rightOf(playerCrawler.rotation);
                turnRight();
                break;

            case kCommandLeft:
                playerCrawler.rotation = leftOf(playerCrawler.rotation);
                turnLeft();
                break;
            case kCommandUp: {
                struct Vec2i offset = mapOffsetForDirection(
                        playerCrawler.rotation);
                gameSnapshot.turn++;

                playerCrawler.position.x += offset.x;
                playerCrawler.position.y += offset.y;


                if (collisionMap[map[playerCrawler.position.y][playerCrawler.position.x]]== '1') {
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

                if (collisionMap[map[playerCrawler.position.y][playerCrawler.position.x]]== '1') {
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
                
                needToRedrawHUD = TRUE;

                while (head2 != NULL && (index < currentSelectedItem)) {
                    ++index;
                    head2 = head2->next;
                }

                if (head2 != NULL) {
                    item2 = head2->item;
                }

                while (head1 != NULL && item1 == NULL) {
                    if (offseted.x == (head1->item->position.x) && offseted.y == (head1->item->position.y)) {
                        item1 = head1->item;
                    }
                    head1 = head1->next;
                }

                if (item2 != NULL) {
                    if (item1 != NULL ) {
                        char buffer[255];
                        char *operator1;
                        char *operand1;
                        
                        sprintf(&buffer[0], "use-with %s %s", item2->description, item1->description);
                        operator1 = strtok(&buffer[0], "\n ");
                        operand1 = strtok(NULL, "\n ");
                        parseCommand(operator1, operand1);
                    } else {
                        parseCommand("use", item2->description);
                    }
                }
                gameSnapshot.turn++;
            }
                break;
            case kCommandFire2: {
                struct ObjectNode *head = getRoom(getPlayerRoom())->itemsPresent->next;
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;

                needToRedrawHUD = TRUE;
                
                while (head != NULL && item == NULL) {
                    if (offseted.x == (head->item->position.x) && offseted.y == (head->item->position.y)) {
                        item = head->item;
                    }
                    head = head->next;
                }

                if (item != NULL) {
                    
                    if (item == getItemNamed("computer-terminal")) {
                        enterState(kHackingGame);
                    } else {
                        parseCommand(item->pickable ? "pick" : "use", item->description);
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
                        item = head->item;
                    }
                    
                    if (item != NULL) {
                        parseCommand("drop", item->description);
                        item->position.x = offseted.x;
                        item->position.y = offseted.y;
                        setItem(item->position.x, item->position.y, item->index);
                        currentSelectedItem = 0;
                    }
                }
                gameSnapshot.turn++;
            }
                break;
            case kCommandFire3: {
                enterState(kInspectItem);
                needToRedrawHUD = TRUE;
            }
                break;
                
            case kCommandFire4: {
                struct ObjectNode *playerItems = getPlayerItems();
                int index = 0;
                
                needToRedrawHUD = TRUE;
                
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

                if (collisionMap[map[playerCrawler.position.y][playerCrawler.position.x]]== '1') {
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

                if (collisionMap[map[playerCrawler.position.y][playerCrawler.position.x]]== '1') {
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

    setActor(playerCrawler.position.x, playerCrawler.position.y, 0xFF);

    if (oldTurn != gameSnapshot.turn) {

        switch (getGameStatus()) {
            case kBadVictory:
                enterState(kBadVictoryEpilogue);
                return gameSnapshot;

            case kBadGameOver:
                enterState(kBadGameOverEpilogue);
                return gameSnapshot;

            case kGoodVictory:
                enterState(kGoodVictoryEpilogue);
                return gameSnapshot;

            case kGoodGameOver:
                enterState(kGoodGameOverEpilogue);
                return gameSnapshot;
        }


        if (getPlayerRoom() != 1 && (!getItemNamed("helmet")->active || !playerHasObject("helmet"))) {
            setPlayerHealth(getPlayerHealth() - 1);
        }

        {
            struct ObjectNode *roomItems = getRoom(getPlayerRoom())->itemsPresent->next;
            struct Item *item = NULL;
            struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
            focusItemName = NULL;
            offseted.x += playerCrawler.position.x;
            offseted.y += playerCrawler.position.y;

            while (roomItems != NULL && item == NULL) {
                if (offseted.x == (roomItems->item->position.x) && offseted.y == (roomItems->item->position.y)) {
                    item = roomItems->item;
                }
                roomItems = roomItems->next;
            }

            if (item != NULL) {
                focusItemName = item->description;
            }
        }
        
        //for the elevators
        if (currentPlayerRoom != getPlayerRoom()) {
            enable3DRendering = FALSE;
            enteredThru = 0 ;
            setPlayerDirection(enteredThru);
            initRoom(getPlayerRoom());
            
            thisMissionName = getRoomDescription();
            thisMissionNameLen = strlen(getRoomDescription());
            
            setPlayerPosition(&oldPosition);
            
            return gameSnapshot;
        }

        cell = map[playerCrawler.position.y][playerCrawler.position.x];

        if ('0' <= cell && cell <= '3') {

            moveBy(cell - '0');

            if (currentPlayerRoom != getPlayerRoom()) {
                int c = 0;
                int room = getPlayerRoom();
                struct Room* roomPtr = getRoom(room);
                
                for (c = 0; c < 6; ++c ) {
                    if (roomPtr->connections[c] == currentPlayerRoom) {
                        enteredThru = oppositeOf(c);
                    }
                }
                
                enable3DRendering = FALSE;
                setPlayerDirection(enteredThru);
                zCameraOffset = intToFix(4);
                currentPresentationState = kRoomTransitioning;
                initRoom(room);
                thisMissionName = getRoomDescription();
                thisMissionNameLen = strlen(thisMissionName);

            } else { /* Something prevented the player from moving - maybe not enough clearance */
                setPlayerPosition(&oldPosition);
                playerCrawler.position.x = oldPosition.x;
                playerCrawler.position.y = oldPosition.y;
                setActor(playerCrawler.position.x, playerCrawler.position.y, 0xFF);
                gameSnapshot.camera_x = playerCrawler.position.x;
                gameSnapshot.camera_z = playerCrawler.position.y;
            }

            return gameSnapshot;
        }

        head = getRoom(getPlayerRoom())->itemsPresent->next;

        while (head != NULL) {
            setItem(head->item->position.x, head->item->position.y, head->item->index);
            head = head->next;
        }
    }

    return gameSnapshot;
}

void dungeon_loadMap(const uint8_t *__restrict__ mapData,
                     const char *__restrict__ collisions,
                     const int mapIndex) {
    int x, y;
    const uint8_t *ptr = mapData;
    struct WorldPosition worldPos;
    struct ObjectNode *head;
    
    gameSnapshot.should_continue = kCrawlerGameInProgress;
    gameSnapshot.mapIndex = mapIndex;
    gameSnapshot.camera_rotation = 0;
    playerCrawler.symbol = '^';
    playerCrawler.rotation = 0;
    memcpy (&collisionMap, collisions, 256);

    for (y = 0; y < MAP_SIZE; ++y) {
        for (x = 0; x < MAP_SIZE; ++x) {
            char current = *ptr;
            map[y][x] = current;
            setItem(x, y, 0xFF);
            setActor(x, y, 0xFF);
            setElement(x, y, current);

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
    setActor(x, y, 0xFF);
    setPlayerPosition(&worldPos);
    playerCrawler.position.x = x;
    playerCrawler.position.y = y;

	head = getRoom(getPlayerRoom())->itemsPresent->next;
    
    while (head != NULL) {
      setItem(head->item->position.x, head->item->position.y, head->item->index);
      head = head->next;
    }
    
    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;
}
