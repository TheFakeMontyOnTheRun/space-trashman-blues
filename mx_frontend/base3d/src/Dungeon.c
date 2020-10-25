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
struct WorldPosition origin;

extern int shouldContinue;

extern char crawlClueMessage[128];

struct CrawlerAgent {
    struct Vec2i position;
    enum EDirection rotation;
    char symbol;
    uint8_t life;
    struct Vec2i target;
    uint8_t ammo;
};

struct CrawlerAgent playerCrawler;

void update_log() {
    char buffer[128];
    char *keysAndTargets;

    if (gameSnapshot.keyCollected) {
        keysAndTargets = "Key collected.\n ";
    } else {
        if (gameSnapshot.targetLocated) {
            keysAndTargets = "Target located. Key still pending.\n ";
        } else {
            keysAndTargets = "";
        }
    }

    sprintf (&buffer[0], "%s", keysAndTargets);
    sprintf (crawlClueMessage, "%s", &buffer[0]);
}

void pursue(struct CrawlerAgent *spy, struct Vec2i target) {

    int dx = (target.x - spy->position.x);
    int dy = (target.y - spy->position.y);
    int x = spy->position.x;
    int y = spy->position.y;
    int incX = 0;
    int incY = 0;
    int iX;
    int iY;

    if (abs(dx) >= abs(dy)) {
        if (dx >= 0) {
            incX = 1;
            spy->rotation = 1;
        } else {
            incX = -1;
            spy->rotation = 3;
        }
    } else {
        if (dy >= 0) {
            incY = 1;
            spy->rotation = 2;
        } else {
            incY = -1;
            spy->rotation = 0;
        }
    }

    iX = x + incX;
    iY = y + incY;


    spy->position.x = iX;
    spy->position.y = iY;


    if (spy->symbol == 'e') {
        spy->symbol = 'f';
    } else {
        spy->symbol = 'e';
    }
}

int isPositionAllowed(int x, int y) {

    return (0 <= x) && (x < MAP_SIZE) && (0 <= y) && (y < MAP_SIZE)
           && collisionMap[map[y][x]] != '1';
}

int isCovered(struct Vec2i position) {
    return (!isPositionAllowed(position.x, position.y - 1))
           || (!isPositionAllowed(position.x, position.y + 1))
           || (!isPositionAllowed(position.x + 1, position.y))
           || (!isPositionAllowed(position.x - 1, position.y));
}

void tickEnemy(struct CrawlerAgent *actor) {
    int x, y;

    struct Vec2i pos0 = actor->position;
    struct Vec2i pos1 = playerCrawler.position;

    int chances = gameSnapshot.detected || ((!gameSnapshot.covered) || ((gameSnapshot.turn % 4) == 0));

    x = actor->position.x;
    y = actor->position.y;

    setActor(x, y, '.');

    if (actor->life == 0) {
        setItem(x, y, '*');
        return;
    }

    if (chances && canSeeSpy(pos0, actor->rotation, pos1, 1)) {
        actor->target = playerCrawler.position;

        /*First detection*/
        if (!gameSnapshot.detected) {
            setDetected();
            gameSnapshot.detected = TRUE;
        }

        /*Move towards player*/
        pursue(actor, actor->target);

        if (!isPositionAllowed(actor->position.x, actor->position.y)
            || (playerCrawler.position.x == actor->target.x &&
                playerCrawler.position.y == actor->target.y)) {
            actor->position.x = x;
            actor->position.y = y;
        }

        if ((gameSnapshot.turn % 2) == 0) {

            /*Shooting at player*/
            addEffectSprite(actor->position.x, actor->position.y, '+');

            /*Bullseye!*/
            if (!gameSnapshot.covered || ((gameSnapshot.turn % 4) == 0)) {
                playerCrawler.life--;
                playerHealth = playerCrawler.life;
                visibilityCached = FALSE;
                needsToRedrawVisibleMeshes = TRUE;

                setDamage();
                if (playerCrawler.life <= 0) {
                    gameSnapshot.should_continue = kCrawlerGameOver;
                }
            }
        }
    } else if (isPositionAllowed(actor->position.x, actor->position.y) &&
               isPositionAllowed(actor->target.x, actor->target.y)) {

        if ((actor->target.x != actor->position.x)
            || (actor->target.y != actor->position.y)) {

            char prevSymbol = actor->symbol;

            pursue(actor, actor->target);

            if (!isPositionAllowed(actor->position.x, actor->position.y) ||
                ((actor->position.x == playerCrawler.position.x)
                 && (actor->position.y == playerCrawler.position.y))) {
                actor->position.x = x;
                actor->position.y = y;
                actor->symbol = prevSymbol;
            }

            if ((abs(actor->position.x - actor->target.x) <= 1)
                && (abs(actor->position.y - actor->target.y) <= 1)) {
                actor->target.x = actor->target.y = -1;
            }
        } else {
            actor->symbol = 'g';
            actor->rotation = rightOf(actor->rotation);
        }
    } else {
        actor->symbol = 'g';
        actor->rotation = leftOf(actor->rotation);
    }

    setActor(actor->position.x, actor->position.y, actor->symbol);
}

struct GameSnapshot dungeon_tick(const enum ECommand command) {

    int oldTurn = gameSnapshot.turn;
    setActor(playerCrawler.position.x, playerCrawler.position.y, '.');

    if (playerCrawler.life > 0) {
        switch (command) {
            case kCommandRight:
                playerCrawler.rotation = rightOf(playerCrawler.rotation);
                break;

            case kCommandLeft:
                playerCrawler.rotation = leftOf(playerCrawler.rotation);
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
                }
                zCameraOffset = -intToFix(2);
            }
                break;
            case kCommandFire1:
                if (playerCrawler.ammo > 0) {
                    /* fire */
                    playerCrawler.ammo--;
                    shootGun();
                }
                break;
            case kCommandFire2: {
                struct ObjectNode* head = getPlayerItems();
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;
                
                if (head != NULL) {
                    item = head->item;
                }
                
                if (item != NULL) {
                    parseCommand("drop", item->description);
                    item->position.x = offseted.x - origin.x;
                    item->position.y = offseted.y - origin.y;
                    setItem(origin.x + item->position.x, origin.y + item->position.y, 'K');
                }

            }
                break;
            case kCommandFire3: {
                struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
                struct Item *item = NULL;
                struct Vec2i offseted = mapOffsetForDirection(playerCrawler.rotation);
                offseted.x += playerCrawler.position.x;
                offseted.y += playerCrawler.position.y;
                
                while (head != NULL && item == NULL) {
                    if (offseted.x == (head->item->position.x + origin.x) && offseted.y == (head->item->position.y + origin.y)) {
                        item = head->item;
                    }
                    head = head->next;
                }
                
                if (item != NULL) {
                    parseCommand("pick", item->description);
                    setItem(offseted.x, offseted.y, '.');
                }

            }

                break;
            case kCommandFire4:
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
                }
                xCameraOffset = intToFix(2);
            }
                break;
            default:
                break;
        }
    } else {
        gameSnapshot.turn++;
    }

    gameSnapshot.camera_x = playerCrawler.position.x;
    gameSnapshot.camera_z = playerCrawler.position.y;
    gameSnapshot.camera_rotation = playerCrawler.rotation;
    gameSnapshot.ammo = playerCrawler.ammo;

    setActor(playerCrawler.position.x, playerCrawler.position.y, '^');

    if (oldTurn != gameSnapshot.turn) {
        
        int cell = map[playerCrawler.position.y][playerCrawler.position.x];
        
    
        if ( '0' <= cell && cell <= '3') {
            moveBy(cell - '0');
            int room = getPlayerRoom();
            initRoom(room);
            origin.x = 0;
            origin.y = 0;

            struct WorldPosition worldPos = getPlayerPosition();
            int x = worldPos.x;
            int y = worldPos.y;
            setActor(x, y, '^');
            playerCrawler.position.x = x;
            playerCrawler.position.y = y;
            
            return gameSnapshot;
        }
        
        struct ObjectNode* head = getRoom(getPlayerRoom())->itemsPresent->next;
        struct Item *item = NULL;
        
        while (head != NULL) {
            setItem(origin.x + head->item->position.x, origin.y + head->item->position.y, 'K');
            head = head->next;
        }
        
        update_log();

        gameSnapshot.covered = isCovered(playerCrawler.position);
    }

    return gameSnapshot;
}

void dungeon_loadMap(const uint8_t *__restrict__ mapData,
                     const char *__restrict__ collisions,
                     const int mapIndex) {
    int x, y;
    const uint8_t *ptr = mapData;
    gameSnapshot.keyCollected = FALSE;
    gameSnapshot.targetLocated = FALSE;
    gameSnapshot.infoCollected = FALSE;
    gameSnapshot.covered = FALSE;
    gameSnapshot.detected = FALSE;
    gameSnapshot.should_continue = kCrawlerGameInProgress;
    gameSnapshot.ammo = 15;
    gameSnapshot.mapIndex = mapIndex;
    gameSnapshot.camera_rotation = 0;
    playerCrawler.symbol = '^';
    playerCrawler.rotation = 0;
    playerCrawler.life = 5;
    playerCrawler.ammo = 15;
    playerHealth = playerCrawler.life;
    memcpy (&collisionMap, collisions, 256);
    origin.x = 0;
    origin.y = 0;

    
    parseCommand("pick", "low-rank-keycard");
    parseCommand("pick", "magnetic-boots");
    parseCommand("pick", "helmet");
    parseCommand("use", "helmet");
    parseCommand("use", "magnetic-boots");

    for (y = 0; y < MAP_SIZE; ++y) {
        for (x = 0; x < MAP_SIZE; ++x) {
            char current = *ptr;
            map[y][x] = current;
            setItem(x, y, '.');
            setActor(x, y, '.');
            setElement(x, y, current);
            
            if (current == '4') {
                origin.x = x;
                origin.y = y;
            }
            ++ptr;
        }
        ++ptr;
    }
    
    struct WorldPosition worldPos = getPlayerPosition();
    x = worldPos.x + origin.x;
    y = worldPos.y + origin.y;
    setActor(x, y, '^');
    playerCrawler.position.x = x;
    playerCrawler.position.y = y;

}
