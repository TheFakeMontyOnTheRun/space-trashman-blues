#include <string>
#include <cstdint>
#include <vector>
#include <sstream>

#include "editor-core.h"



namespace editor {

  uint8_t Map::getConnection(int slot) {
    return connections[slot];
  }

  void Map::setConnetion(int slot, uint8_t connection) {
    connections[slot] = connection;
  }

  void Map::setName(const std::string& newName) {
    name = newName;
  }
  
  std::string Map::getName() {
    return name;
  }

  void saveMap(const Map& map) {
  }

  uint32_t Map::createArea(uint8_t x0, uint8_t y0, uint8_t z0, uint8_t dx, uint8_t dy, uint8_t dz) {
    areas.push_back({x0, y0, z0, dx, dy, dz});
    return 0;
  }

  std::vector<uint32_t> Map::getAreaIds() {
    std::vector<uint32_t> ids;


    return ids;
  }

  const Area& Map::getArea(const uint32_t id) {
    return areas[id];
  }
}