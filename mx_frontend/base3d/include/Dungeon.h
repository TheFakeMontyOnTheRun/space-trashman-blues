#ifndef HASHASHIN_DUNGEON_H
#define HASHASHIN_DUNGEON_H

struct GameSnapshot {
	uint8_t camera_x;
	uint8_t camera_z;
	uint8_t camera_rotation;
	int detected;
	enum CrawlerState should_continue;
	int ammo;
	int turn;
	int covered;
	int keyCollected;
	int infoCollected;
	int mapIndex;
	int targetLocated;
	struct Vec2i playerTarget;
};

struct GameSnapshot dungeon_tick(const enum ECommand cmd);

void dungeon_loadMap(
		const uint8_t * __restrict__ mapData,
		const char * __restrict__ collisions,
		const int map);

void addEffectSprite(const int x, const int y, const int billboard);

int canSeeSpy(const struct Vec2i seer,
			  int direction,
			  const struct Vec2i target,
			  int enemy);

void setElement(const int x, const int y, const uint8_t element);

void setActor(const int x, const int y, const uint8_t actor);

void setItem(const int x, const int y, const uint8_t item);

void setDamage(void);

void setDetected(void);
#endif
