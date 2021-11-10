/*
Created by Daniel Monteiro on 2019-07-26.
*/


#ifndef DONT_INCLUDE

#ifndef SMD
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#include "Win32Int.h"
#else
#include <stdint.h>
#endif

#ifndef CPC_PLATFORM
#ifndef NGB
#include <unistd.h>
#endif
#endif

#else
#include <genesis.h>
#endif

#include "Core.h"
#include "Derelict.h"
#endif

int accessGrantedToSafe = FALSE;

void updateRankFromKeycards() {
    int rank = 0;
    
    if (playerHasObject("low-rank-keycard")) {
        rank = 1;
    }

    if (playerHasObject("hacked-keycard")) {
        rank = 2;
    }

    if (playerHasObject("high-rank-keycard")) {
        rank = 3;
    }

    if (playerHasObject("root-keycard")) {
        rank = 4;
    }

    setPlayerRank(rank);
}

void keycardPickCallback(struct Item *item) {
    updateRankFromKeycards();
}


void keycardDropCallback(struct Item *item) {
    updateRankFromKeycards();
}



void useCardWithCardWritter(struct Item *item1, struct Item *item2) {
    if (item2 == getItemNamed("card-writter")) {
        struct Item* card = getItemNamed("hacked-keycard");
        addToRoom("computer-core", card);
        dropObjectByName("low-rank-keycard");
        removeObjectFromRoom(getItemNamed("low-rank-keycard"));
    } else {
        defaultLogger("No effect");
    }
}

void useBootsWithMagneticCoupling(struct Item *item1, struct Item *item2) {
    struct Item *coupling = getItemNamed("magnetic-coupling");
    if (item2 == coupling ) {
        coupling->active = FALSE;
        defaultLogger("Magnetic lock disengaged");
    } else {
        defaultLogger("No effect");
    }
}

/*
 Good victory - you blew the station and escaped
 Bad victory - you blew the station, but died
 good game over - you escaped, but failed to blow the station
 bad game over - you failed to blow the station and died.
 */
void bombActivatedCallback(struct Item *item) {

    int empOnReactor = hasItemInRoom( "reactor-core", "emp-bomb");
    int playerLocation = getPlayerRoom();
    int playerAtDaedaus = (playerLocation == 1);
    int playerAtSameLocationAsBomb = hasItemInRoom( getRoom( playerLocation)->description, "emp-bomb");
    
    if (empOnReactor) {
        if (playerAtDaedaus) {
            setGameStatus(kGoodVictory);
        } else {
            setGameStatus(kBadVictory);
        }
    } else {
        if (playerAtSameLocationAsBomb) {
            setGameStatus(kBadGameOver);
        } else {
            if (playerAtDaedaus  ) {
                setGameStatus(kGoodGameOver);
            } else {
                setGameStatus(kBadGameOver);
            }
        }
    }
}

void bombControllerActivatedCallback(struct Item *item) {
    bombActivatedCallback(NULL);
}

void elevatorGoDownCallback(struct Item *item) {
    struct ObjectNode* currentItem;
    struct ObjectNode* nextItem;
    int newRoom;

    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }
    
    newRoom = getRoom(getPlayerRoom())->connections[4];
    currentItem = getRoom(getPlayerRoom())->itemsPresent->next;
    
    while(currentItem != NULL && getItem(currentItem->item) != NULL ) {
        
        nextItem = currentItem->next;
        
        if (getItem(currentItem->item)->pickable) {
            item = getItem(currentItem->item);
            removeObjectFromRoom(item);
            addObjectToRoom( newRoom, item);
        }
        
        currentItem = nextItem;
    }
    
    moveBy(4);
}

void elevatorGoUpCallback(struct Item *item) {
    
    struct ObjectNode* currentItem;
    struct ObjectNode* nextItem;
    int newRoom;
    
    if (!getItemNamed("comm-terminal-2")->active) {
        defaultLogger("Central computer is offline");
        return;
    }

    newRoom = getRoom(getPlayerRoom())->connections[5];
    currentItem = getRoom(getPlayerRoom())->itemsPresent->next;
    
    while(currentItem != NULL && getItem(currentItem->item) != NULL) {
 
        nextItem = currentItem->next;
        
        if (getItem(currentItem->item)->pickable) {
            item = getItem(currentItem->item);
            removeObjectFromRoom(item);
            addObjectToRoom( newRoom, item);
        }
        
        currentItem = nextItem;
    }

    moveBy(5);
}

void useCloggedFlush(struct Item *item) {
    struct Item* highRankKeycard = getItemNamed("high-rank-keycard");
    if (highRankKeycard->roomId == 0) {
        defaultLogger("Found something among the\n...stuff...");
        addToRoom("wc", highRankKeycard);
    }
}

void useRegularFlush(struct Item *item) {
    defaultLogger("*FLUSH*");
}


void cantBeUsedCallback(struct Item *item) {
    defaultLogger("You can't use it like this.");
}

void cantBeUsedWithOthersCallback(struct Item *item1, struct Item *item2) {
    defaultLogger("Nothing happens.");
}

void useObjectToggleCallback(struct Item *item) {
    item->active = !item->active;
}

void useCommWithRank(struct Item *item) {
    
    if (getPlayerRank() <= 1) {
        defaultLogger("Insufficient rank to access");
        return;
    }
    defaultLogger("Computer core rebooted");
    item->active = !item->active;
}


void useComputerRack(struct Item *item) {
    
    if (accessGrantedToSafe) {
        defaultLogger("Safe unlocked");
        addToRoom("situation-room", getItemNamed("root-keycard"));
        return;
    }
    
    defaultLogger("Safe secured");
}

void reactorValveCallback(struct Item *item) {
    setGameStatus(kBadVictory);
}


void initStation(void) {

    struct Item* newItem;
    int connections[6];
    accessGrantedToSafe = FALSE;

    initCore();

    /*Rooms*/
    /* 1 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 2;
    addRoom(
            "lss-daedalus",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "The salvage operations vehicle. It's\n"
            "tracked remotely, to prevent \n"
            "escapes. If I try taking outside \n"
            "the predicted path, I will sink into\n"
            "the abyss myself.",
#endif
            32, 32, 0, connections);
    
    /* 2 */
    connections[2] = 1;
    connections[1] = 6;
    connections[0] = 3;
    addRoom("hangar",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "The main hangar is rather \n"
            "unremarkable. The only thing you \n"
            "notice is a slight yellow tint of \n"
            "the air, as if a mist slides next \n"
            "to the floor. It's very faint. Your\n"
            "ship's computer tells you this is\n"
            "harmless (as if those readings were\n"
            "worth the trust). Unfortunately, no\n"
            "useful tools around here. Around the\n"
            "corner, there's a escape pod entrance.\n"
            "Apparently, only one pod was launched.",
#endif
            32, 32, 0, connections);

    /* 3 */
    connections[2] = 0;
    connections[3] = 2;
    connections[0] = 4;
    connections[1] = 5;
    addRoom("hall-2",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A well lit hall, with doors. It's \n"
            "the main hub of the vehicle. Despite\n"
            "being right next to the hangar and\n"
            "the control room, it's rather quiet.",
#endif
            32, 32, 0, connections);

    /* 4 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 3;
    connections[4] = 19;
    connections[5] = 13;
    addRoom("elevator-level-2",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "It's rather surprising that this \n"
            "ship has an elevator. This is was\n"
            "typical only of ships with 5 levels\n"
            "or more.",
#endif
            64, 64, 0, connections)->rankRequired = 1;
    
    /* 5 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 3;
    connections[1] = 9;
    connections[0] = 7;
    connections[2] = 8;
    addRoom("dorms-1",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "Part of the dorms hallway. There are\n"
            "some (busted) control panels for \n"
            "ejecting the pods. Some pieces of \n"
            "cloth and broken plastic on the floor,\n"
            "but nothing really useful.",
#endif
            32, 32, 0, connections)->rankRequired = 1;

    
    /* 6 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[3] = 2;
    addRoom("rls-bohr-2",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A rescue ship. Only for emergencies.\n"
            "Named after some Niels Bohr scientist\n"
            "guy or whatever. Some drops on the\n"
            "carpet and I don't even want know \n"
            "what it is, but I guess I already\n"
            "know. Ick.",
#endif
            64, 9, 0, connections);

    
    /* 7 */
    connections[3] = 0;
    connections[2] = 5;
    addRoom("pod-1",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A living pod. Looks like from one \n"
            "of the oficcers. It's messy, but \n"
            "as if it's occupant would easily \n"
            "find his belongings in there. \n"
            "There are some burn marks on the \n"
            "walls.",
#endif
            32, 32, 0, connections)->rankRequired = 1;
    
    /* 8 */
    connections[2] = 0;
    connections[0] = 5;
    addRoom("pod-2",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A empty living pod. Looks as if it\n"
            "was never ever used. If can even \n"
            "see some of the factory stickers \n"
            "in it.",
#endif
            32, 32, 0, connections)->rankRequired = 2;
    
    /* 9 */
    connections[1] = 12;
    connections[3] = 5;
    connections[0] = 10;
    connections[2] = 11;
    addRoom("dorms-2",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "Anonther part of the dorms hallway.\n"
            "On those, the panels were visibly\n"
            "well. These parts of the quarters \n"
            "were probably the more prestigious ones.",
#endif
            32, 32, 0, connections);
    
    /* 10 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[2] = 9;
    addRoom("pod-3",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A young woman's pod. You \n"
            "do recognize a few items, but its \n"
            "badly mixed up. It's hard to make \n"
            "the age of girl, but she was young.",
#endif
            32, 32, 0, connections)->rankRequired = 3;
    
    /* 11 */
    connections[2] = 0;
    connections[0] = 9;
    addRoom("pod-4",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "A the first officer's pod, for sure.\n"
            "It's neat, clean and organized. Not \n"
            "much around. He had a strange \n"
            "fixation on redheads.",
#endif
            32, 32, 0, connections)->rankRequired = 4;
    
    /* 12 */
    connections[0] = 0;
    connections[3] = 9;
    addRoom("computer-core",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections);

    /* 13 */
    connections[3] = 0;
    connections[4] = 4;
    connections[2] = 14;
    addRoom("elevator-level-1",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "It's rather surprising that this ship\n"
            "has an elevator. This is was typical \n"
            "only of ships with 5 levels or more.",
#endif
            32, 32, 0, connections)->rankRequired = 1;
    
    /* 14 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 13;
    connections[1] = 17;
    connections[2] = 16;
    connections[3] = 15;
    addRoom("hall-1",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "This hall has a busier feel.\n"
            "Here you see objects thrown all over \n"
            "the place, as if someone was in the \n"
            "middle of a day-to-day routine and had\n"
            "to quickly run.",
#endif
            32, 32, 0, connections)->rankRequired = 1;
    
    /* 15 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[1] = 14;
    addRoom("bridge",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections)->rankRequired = 4;
    
    /* 16 */
    connections[1] = 0;
    connections[0] = 14;
    addRoom("situation-room",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections)->rankRequired = 3;

    /* 17 */
    connections[0] = 0;
    connections[3] = 14;
    connections[1] = 18;
    addRoom("crew-bunks",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections)->rankRequired = 1;
    
    /* 18 */
    connections[1] = 0;
    connections[3] = 17;
    addRoom("armory",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD",
#endif
                    32, 32, 0, connections)->rankRequired = 3;

    /* 19 */
    connections[3] = 0;
    connections[2] = 20;
    connections[5] = 4;
    addRoom("elevator-level-3",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "It's rather surprising that this ship has an\n"
            "elevator. This is was typical only of ships \n"
            "with 5 levels or more.",
#endif
            32, 32, 0, connections)->rankRequired = 1;
    
    /* 20 */
    memset(&connections[0], 0, 6 * sizeof(int));
    connections[0] = 19;
    connections[1] = 21;
    connections[3] = 23;
    addRoom("hall-3",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD",
#endif
                    32, 32, 0, connections);

    /* 21 */
    connections[0] = 0;
    connections[3] = 20;
    connections[1] = 22;
    addRoom("wc",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections);
    
    /* 22 */
    connections[1] = 0;
    connections[3] = 21;
    addRoom("reactor-core",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD.",
#endif
                    32, 32, 0, connections)->rankRequired = 4;
    
    /* 23 */
    connections[3] = 0;
    connections[1] = 20;
    addRoom("radar-array",
#ifdef INCLUDE_ROOM_DESCRIPTIONS
            "TBD",
#endif
                    32, 32, 0, connections)->rankRequired = 2;


    /*Items*/
        newItem = addItem("dummy", "ERROR",
    #ifdef ITEMS_HAVE_WEIGHT
                                      0,
    #endif
                                      FALSE, 0, 0);
    
    /* LSS-Daedalus */
    newItem = addItem("emp-bomb", "Time-programmable Halogen EMP bomb.\n"
                                  "Will disable any electrical device\n"
                                  "within the 50 nautical miles range.",
#ifdef ITEMS_HAVE_WEIGHT
                                  5,
#endif
                                  TRUE, 9, 6);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombActivatedCallback;

    
    newItem = addItem("emp-controller",
                      "The remote controller allows you to\n"
                      "instantly detonate the bomb from very\n"
                      "far (empirical evidence tells it \n"
                      "works from as far as 200 nautical\n"
                      "miles).",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 10, 6);
    addToRoom("lss-daedalus", newItem);
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->useCallback = bombControllerActivatedCallback;
                                        
    newItem = addItem("ship-ignition",
                      "Token needed to ignite the ship's\n"
                      "computer and thrusters",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 11, 6);
    addToRoom("lss-daedalus", newItem);
    newItem->useCallback = bombActivatedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;

    newItem = addItem("magnetic-boots",
                      "Boots with strong electro-magnets.\n"
                      "Ideal for walking underwater...\n"
                      "...as long as the surface in question\n"
                      "is metallic (like most of the \n"
                      "surfaces here).",
#ifdef ITEMS_HAVE_WEIGHT
                      2,
#endif
                      TRUE, 15, 15);
    
    newItem->active = TRUE;
    pickObject(newItem);
    newItem->useWithCallback = useBootsWithMagneticCoupling;
    newItem->useCallback = useObjectToggleCallback;
    

    newItem = addItem("helmet",
                      "Atmosphere-contained helmet for\n"
                      "safety.",
#ifdef ITEMS_HAVE_WEIGHT
                      2,
#endif
                      TRUE, 15, 15);
    newItem->active = TRUE;
    pickObject(newItem);
    newItem->useCallback = useObjectToggleCallback;

    
    newItem = addItem("low-rank-keycard",
                      "Clearance for low rank. Oddly, \n"
                      "this one is of the rewrittable kind;\n"
                      "probably due to a field promotion.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 4, 7);
    addToRoom("hall-2", newItem);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = useCardWithCardWritter;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    
    
    newItem = addItem("hacked-keycard",
                      "Hacked keycard for mid-clearance\nrank.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 21, 9);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;


    /* Hangar */
    
    newItem = addItem("magnetic-coupling",
                      "Automatic seal activated by \nspecial safety protocols",
#ifdef ITEMS_HAVE_WEIGHT
                      17,
#endif
                      FALSE, 2, 2);
    addToRoom("hangar", newItem);
    newItem->active = TRUE;
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    
    /* Comm terminals*/
    
    newItem = addItem("door-panel",
                      "Special control for the door",
#ifdef ITEMS_HAVE_WEIGHT
                      200,
#endif
                      FALSE, 12, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hangar", newItem);
    

    /* Comm terminals*/
    newItem = addItem("comm-terminal-1",
                      "Terminal for communicating with\nthe central computer.",
#ifdef ITEMS_HAVE_WEIGHT
                      200,
#endif
                      FALSE, 6, 2);
    newItem->useCallback = useObjectToggleCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-1", newItem);

    
    newItem = addItem("comm-terminal-2",
                      "Terminal for communicating with\nthe central computer.",
#ifdef ITEMS_HAVE_WEIGHT
                      200,
#endif
                      FALSE, 6, 2);
    newItem->useCallback = useCommWithRank;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-2", newItem);
    
    
    newItem = addItem("comm-terminal-3",
                      "Terminal for communicating with\nthe central computer.",
#ifdef ITEMS_HAVE_WEIGHT
                      200,
#endif
                      FALSE, 6, 2);
    newItem->useCallback = useObjectToggleCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("hall-3", newItem);

    /* Diaries */
    newItem = addItem("black-diary",
                      "...We meet every night in the rest\n"
                      "room, to make out. I asked her for\n"
                      "an access key for the armory - let's\n"
                      " see if she keeps her promisse. If\n"
                      "they catch me, I'm scr...",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 4, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-1", newItem);
    
    
    newItem = addItem("blue-diary",
                      "The growing discontent is very \n"
                      "noticeable. I don't know for how\n"
                      "long can we keep the situation\n"
                      "stable. For safety, I gave the root\n"
                      "keycard to first officer Costa.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 11, 8);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-2", newItem);

    

    newItem = addItem("white-diary",
                      "Crew is growing demotivated with\n"
                      "all the combat exercises and no \n"
                      "downtime. Don't know long can I\n"
                      "keep the fact that we already lost\n"
                      "the war. If anything goes wrong,\n"
                      "the situation room will be our\n"
                      "last stand.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 3, 2);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-3", newItem);
    
    
    newItem = addItem("yellow-book",
                      "Situation is hopeless. I must enact\n"
                      "the security lockdown protocol and \n"
                      "set the reactor to a low activity \n"
                      "state. With luck, they will rescue \n"
                      "us in the next weeks or so.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 5, 10);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("pod-4", newItem);

    
    newItem = addItem("log-book",
                      "Power conduit on level 3 was restored\n"
                      "without incidents.",
#ifdef ITEMS_HAVE_WEIGHT
                      1,
#endif
                      TRUE, 26, 8);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("crew-bunks", newItem);

    /* Misc */
    newItem = addItem("card-writter",
                      "Terminal with card writter, connected\n"
                      " to the main computer",
#ifdef ITEMS_HAVE_WEIGHT
                      3,
#endif
                      FALSE, 22, 10);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("computer-core", newItem);

    
    newItem = addItem("high-rank-keycard",
                      "Clearance for high-rank officer.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 23, 17);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    

    newItem = addItem("digital-safe",
                      "A very valuable vintage rare and in\n"
                      "working-conditions computer rack!",
#ifdef ITEMS_HAVE_WEIGHT
                      138,
#endif
                      FALSE, 16, 4);
    newItem->useCallback = useComputerRack;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);
    
    newItem = addItem("computer-terminal",
                      "An offline terminal connected to the\n"
                      "computer node",
#ifdef ITEMS_HAVE_WEIGHT
                      138,
#endif
                      FALSE, 16, 5);
    newItem->useCallback = useComputerRack;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);

    
    newItem = addItem("journal",
                      "...and so you guys could just join in\n"
                      "and see whats going on. I hope it is not\n"
                      "too instrusive of me. To that, she just\n"
                      "gave me a cold stare and...",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 8, 6);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("situation-room", newItem);
    
    newItem = addItem("metal-mending",
                      "A piece of metal that might be valuable.",
#ifdef ITEMS_HAVE_WEIGHT
                      74,
#endif
                      FALSE, 5, 4);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("radar-array", newItem);

    
    newItem = addItem("scientific-treatise",
                      "Voynich Manuscript\nAnnottated Translation.\n"
                      "Classical edition. It's badly burn't.\n"
                      "Can't read it.",
#ifdef ITEMS_HAVE_WEIGHT
                      1,
#endif
                      TRUE, 17, 17);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);
    
    
    newItem = addItem("clogged-flush",
                      "There is so much matter in the pipe...",
#ifdef ITEMS_HAVE_WEIGHT
                      1,
#endif
                      FALSE, 22, 17);
    newItem->useCallback = useCloggedFlush;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);
    
    newItem = addItem("flush",
                      "Working dispose valve for a very basic need.",
#ifdef ITEMS_HAVE_WEIGHT
                      1,
#endif
                      FALSE, 18, 17);
    newItem->useCallback = useRegularFlush;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("wc", newItem);

    
    newItem = addItem("fuel-rods",
                      "This is the source of all the trouble.\n"
                      "Both now and then. Gotta find a way to\n"
                      "eject those into the abyss.",
#ifdef ITEMS_HAVE_WEIGHT
                      209,
#endif
                      FALSE, 29, 10);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("reactor-core", newItem);

    
    newItem = addItem("reactor-valve-control",
                      "This is the computer node that could be\n"
                      "used to eject the rods into the abyss.",
#ifdef ITEMS_HAVE_WEIGHT
                      62,
#endif
                      FALSE, 21, 4);
    newItem->useCallback = reactorValveCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("reactor-core", newItem);

    
    newItem = addItem("root-keycard",
                      "Card for root access.",
#ifdef ITEMS_HAVE_WEIGHT
                      0,
#endif
                      TRUE, 16, 3);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    newItem->pickCallback = keycardPickCallback;
    newItem->dropCallback = keycardDropCallback;
    
    /* Elevator controls */
    newItem = addItem("elevator-level1-go-down", "Elevator controls - Go down.",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                    FALSE, 2, 0);
    newItem->useCallback = elevatorGoDownCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-1", newItem);

    newItem = addItem("elevator-level2-go-down", "Elevator controls - Go down.",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                    FALSE, 2, 0);
    newItem->useCallback = elevatorGoDownCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-2", newItem);
    
    newItem = addItem("elevator-level2-go-up", "Elevator controls - Go Up.",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                    FALSE, 3, 0);
    newItem->useCallback = elevatorGoUpCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-2", newItem);
    
    newItem = addItem("elevator-level3-go-up", "Elevator controls - Go Up.",
#ifdef ITEMS_HAVE_WEIGHT
            0,
#endif
                    FALSE, 3, 0);
    newItem->useCallback = elevatorGoUpCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("elevator-level-3", newItem);

    newItem = addItem("the-mistral-report", "RetroZaragoza Homebrew '18 awarded\nentry: A espionage-themed \nturn-based 3D RPG for MS-DOS,\nXBox One, Mac and Amiga.",
#ifdef ITEMS_HAVE_WEIGHT
            200,
#endif
                    TRUE, 15, 19);
    newItem->useCallback = cantBeUsedCallback;
    newItem->useWithCallback = cantBeUsedWithOthersCallback;
    addToRoom("crew-bunks", newItem);
}
