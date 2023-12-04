#ifndef EDITOR_CORE_H
#define EDITOR_CORE_H

struct Position2 {
  uint8_t x;
  uint8_t y;
};

struct Position3 {
  uint8_t x;
  uint8_t y;
  uint8_t z;
};

struct Area {

  uint8_t x0;
  uint8_t y0;
  uint8_t dx;
  uint8_t dy;
  uint8_t ceilingHeight;
};

struct Light {
  Position3 pos;
  uint8_t intensity;
  uint32_t colour;

};


class Map {
  const static auto kMapSize = 32;
  std::vector<Area> areas;
  std::vector<Light>lights;
  uint8_t rawMap[kMapSize][kMapSize];
  uint8_t connections[6];
  std::string name;
public:
  void createArea(uint8_t x0, uint8_t y0, uint8_t dx, uint8_t dy, uint8_t ceilingHeight);
  uint8_t getConnection(int slot);
  void setConnetion(int slot, uint8_t connection);
  void setName(const std::string& name);
  std::string getName();
};

void saveMap(const Map& map);

#endif // EDITOR_CORE_H
