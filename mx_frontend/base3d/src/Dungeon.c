#include <stdio.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "FixP.h"
#include "Vec.h"
#include "Enums.h"
#include "Dungeon.h"
#include "Common.h"
#include "EDirection_Utils.h"
#include "Engine.h"
#include "SoundSystem.h"
#include "Derelict.h"
#include "Parser.h"

/* This include must be here just to satisfy the .h - your IDE might trick you into thinking this is not needed. And it's not, but ISO requires. */
#include "CActor.h"

#include "CRenderer.h"

struct GameSnapshot gameSnapshot;
uint8_t map[MAP_SIZE][MAP_SIZE];
uint8_t collisionMap[256];
int enteredThru = 0;
extern char* focusItemName;

int currentSelectedItem = 0;

extern int shouldContinue;

extern char crawlClueMessage[128];

struct CrawlerAgent {
    struct Vec2i position;
    enum EDirection rotation;
    char symbol;
    uint8_t life;
    struct Vec2i target;
};

struct CrawlerAgent playerCrawler;

int isPositionAllowed(int x, int y) {

    return (0 <= x) && (x < MAP_SIZE) && (0 <= y) && (y < MAP_SIZE)
           && collisionMap[map[y][x]] != '1';
}

struct GameSnapshot dungeon_tick(const enum ECommand command) {

    int oldTurn = gameSnapshot.turn;
    struct WorldPosition oldPosition = getPlayerPosition();
    setActor(playerCrawler.position.x, playerCrawler.position.y, '.');
    int currentPlayerRoom = getPlayerRoom();
    
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


                if (collisionMap[map[playerCrawler.position.y]
                [playerCrawler.position.x]]
                    == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(0);
                }
                zCameraOffset = intToFix(2);
            }
                break;
            case kCommandDown: {
                struct Vec2i offset = mapOffsetForDirection(
                        playerCrawler.rotation);
                gameSnapshot.turn++;
                playerCrawler.position.x -= offset.x;
                playerCrawler.position.y -= offset.y;

                if (collisionMap[map[playerCrawler.position.y]
                [playerCrawler.position.x]]
                    == '1') {
                    playerCrawler.position.x += offset.x;
                    playerCrawler.position.y += offset.y;
                } else {
                    walkBy(2);
                }
                zCameraOffset = -intToFix(2);
            }
                break;
                
            case kCommandFire5: {
                struct ObjectNode* head = getPlayerItems();
                struct Item *item = NULL;
                int index = 0;
                
                ++currentSelectedItem;
                
                while (head != NULL) {
                    ++index;
                    head = head->next;
                }
                
                if ( currentSelectedItem > index ) {
                    currentSelectedItem = 0;
                }
            } break;
                
            case kCommandFire4: {
                struct ObjectNode* head1 = getRoom(getPlayerRoom())->itemsPresent->next;
                struct Item *item1 = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                struct ObjectNode* head2 = getPlayerItems();
                struct Item *item2 = NULL;
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;
                
                int index = 0;
                
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
                
                if (item1 != NULL && item2 != NULL) {
                    char buffer[255];
                    sprintf(&buffer[0], "use-with %s %s", item2->description, item1->description);
                    char *operator1 = strtok( &buffer[0], "\n " );
                    char *operand1 = strtok( NULL, "\n ");
                    parseCommand(operator1, operand1);
                }
                gameSnapshot.turn++;
            } break;
            case kCommandFire2: {
                struct ObjectNode* head = getPlayerItems();
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;
                
                int index = 0;
                
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
                    setItem(item->position.x, item->position.y, 'K');
                    currentSelectedItem--;
                }
            }
            case kCommandFire1: {
                struct ObjectNode* head = getPlayerItems();
                struct Item *item = NULL;
                int index = 0;
                
                while (head != NULL && (index < currentSelectedItem)) {
                    ++index;
                    head = head->next;
                }
                
                if (head != NULL) {
                    item = head->item;
                }
                
                if (item != NULL) {
                    parseCommand("use", item->description);
                }
                gameSnapshot.turn++;
            } break;
                
            case kCommandFire3: {
                struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;
                
                while (head != NULL && item == NULL) {
                    if (offseted.x == (head->item->position.x) && offseted.y == (head->item->position.y)) {
                        item = head->item;
                    }
                    head = head->next;
                }
                
                if (item != NULL) {
                    parseCommand( item->pickable ? "pick" : "use", item->description);
                    if (item->pickable) {
                        setItem(offseted.x, offseted.y, '.');
                    }
                }
                gameSnapshot.turn++;
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

                if (collisionMap[map[playerCrawler.position.y]
                [playerCrawler.position.x]]
                    == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(3);
                }
                xCameraOffset = -intToFix(2);
            }
                break;
            case kCommandStrafeRight: {
                struct Vec2i offset =
                        mapOffsetForDirection(rightOf(playerCrawler.rotation));
                gameSnapshot.turn++;

                playerCrawler.position.x += offset.x;
                playerCrawler.position.y += offset.y;

                if (collisionMap[map[playerCrawler.position.y]
                [playerCrawler.position.x]]
                    == '1') {
                    playerCrawler.position.x -= offset.x;
                    playerCrawler.position.y -= offset.y;
                } else {
                    walkBy(1);
                }
                xCameraOffset = intToFix(2);
            }
                break;
            default:
                break;
        }
    }

    struct WorldPosition worldPos = getPlayerPosition();
    playerCrawler.position.x = worldPos.x;
    playerCrawler.position.y = worldPos.y;
    setActor(playerCrawler.position.x, playerCrawler.position.y, '^');
    playerCrawler.rotation = getPlayerDirection();
    
    gameSnapshot.camera_x = playerCrawler.position.x;
    gameSnapshot.camera_z = playerCrawler.position.y;
    gameSnapshot.camera_rotation = playerCrawler.rotation;

    setActor(playerCrawler.position.x, playerCrawler.position.y, '^');

    if (oldTurn != gameSnapshot.turn) {
        
        if (getPlayerRoom() != 1 && (!getItem(6)->active || !playerHasObject("helmet"))) {
            setPlayerHealth(getPlayerHealth() - 1);
        }
        
        {
            focusItemName = NULL;
            struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
            struct Item *item = NULL;
            struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
            offseted.x += playerCrawler.position.x;
            offseted.y += playerCrawler.position.y;
            
            while (head != NULL && item == NULL) {
                if (offseted.x == (head->item->position.x) && offseted.y == (head->item->position.y)) {
                    item = head->item;
                }
                head = head->next;
            }
            
            if (item != NULL) {
                focusItemName = item->description;
            }
        }
        
        int cell = map[playerCrawler.position.y][playerCrawler.position.x];
        
        
        if (currentPlayerRoom != getPlayerRoom()) {
            enable3DRendering = FALSE;
            enteredThru = 0 ;
            setPlayerDirection(enteredThru);
            initRoom(getPlayerRoom());
            
            return gameSnapshot;
        }
        
    
        if ( '0' <= cell && cell <= '3') {
            
            moveBy(cell - '0');
            
            if (currentPlayerRoom != getPlayerRoom()) {
                enable3DRendering = FALSE;
                int room = getPlayerRoom();
                enteredThru = cell - '0';
                setPlayerDirection(enteredThru);
                initRoom(room);
            } else {
                setPlayerPosition(oldPosition);
                playerCrawler.position.x = oldPosition.x;
                playerCrawler.position.y = oldPosition.y;
                setActor(playerCrawler.position.x, playerCrawler.position.y, '^');
                gameSnapshot.camera_x = playerCrawler.position.x;
                gameSnapshot.camera_z = playerCrawler.position.y;
                setActor(playerCrawler.position.x, playerCrawler.position.y, '^');
            }
            
            return gameSnapshot;
        }
        
        struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
        
        while (head != NULL) {
            setItem(head->item->position.x, head->item->position.y, 'K');
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
    gameSnapshot.should_continue = kCrawlerGameInProgress;
    gameSnapshot.mapIndex = mapIndex;
    gameSnapshot.camera_rotation = 0;
    playerCrawler.symbol = '^';
    playerCrawler.rotation = 0;
    memcpy (&collisionMap, collisions, 256);
    
    struct WorldPosition worldPos;

    for (y = 0; y < MAP_SIZE; ++y) {
        for (x = 0; x < MAP_SIZE; ++x) {
            char current = *ptr;
            map[y][x] = current;
            setItem(x, y, '.');
            setActor(x, y, '.');
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
    setActor(x, y, '^');
    setPlayerPosition(worldPos);
    playerCrawler.position.x = x;
    playerCrawler.position.y = y;
    

    visibilityCached = FALSE;
    needsToRedrawVisibleMeshes = TRUE;
}
