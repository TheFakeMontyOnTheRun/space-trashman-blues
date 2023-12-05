#ifndef EDITOR_CORE_H
#define EDITOR_CORE_H

namespace editor {
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
    uint8_t z0;
    uint8_t dx;
    uint8_t dy;
    uint8_t dz;
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
    void createArea(uint8_t x0, uint8_t y0, uint8_t z0, uint8_t x1, uint8_t y1, uint8_t z1);
    uint8_t getConnection(int slot);
    void setConnetion(int slot, uint8_t connection);
    void setName(const std::string& name);
    std::string getName();


   friend std::string to_string(const Map& map) {
     char cellMap[32][32];
     std::stringstream ss;
     
     for (int y = 0; y < 32; ++y ) {
       for (int x = 0; x < 32; ++x){
	 cellMap[y][x] = '.';
       }
     }
     
     for (const auto& area : map.areas) {
       
       for (int x = area.x0 + 1; x < (area.x0 + area.dx); ++x){
	 cellMap[area.z0][x] = '#';
	 cellMap[(area.z0 + area.dz)][x] = '#';
       }
       
       for (int y = area.z0 + 1; y < (area.z0 + area.dz); ++y ) {
	 cellMap[y][area.x0] = '#';
	 cellMap[y][(area.x0 + area.dx)] = '#';
	 for (int x = area.x0 + 1; x < (area.x0 + area.dx); ++x){
	   cellMap[y][x] = '_';
	 }
       }
     }
     
     for (int y = 0; y < 32; ++y ) {
       for (int x = 0; x < 32; ++x){
	 ss << cellMap[y][x];
       }
       ss << "\n";
     }
     
     return ss.str();
   }
  };
  
  void saveMap(const Map& map);

  std::string to_string(const Map& map);
}
#endif // EDITOR_CORE_H
