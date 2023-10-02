#ifndef HASHASHIN_DUNGEON_H
#define HASHASHIN_DUNGEON_H

struct GameSnapshot {
    int8_t camera_x;
    uint8_t camera_z;
    uint8_t camera_rotation;
    enum CrawlerState should_continue;
    int turn;
    struct Vec2i playerTarget;
};

struct GameSnapshot dungeon_tick(const enum ECommand cmd);

void dungeon_loadMap(
        const uint8_t *__restrict__ mapData,
        const char *__restrict__ collisions,
        const int map);


void setItem(const int x, const int y, const uint8_t item);

void initRoom(int room);

extern int enteredThru;

extern int cursorPosition;

extern const char *focusItemName;

extern struct GameSnapshot gameSnapshot;

#endif
