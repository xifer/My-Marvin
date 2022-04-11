#include "Map.h"

#include <fstream>

#include "RegionRegistry.h"

namespace marvin {

Map::Map(const TileData& tile_data) : tile_data_(tile_data) {}

TileId Map::GetTileId(u16 x, u16 y) const {
  if (x >= 1024 || y >= 1024) return 0;
  return tile_data_[y * kMapExtent + x];
}

bool Map::IsSolid(u16 x, u16 y) const {
  if (x >= 1024 || y >= 1024) return true;
  return IsSolid(GetTileId(x, y));
}

void Map::SetTileId(u16 x, u16 y, TileId id) {
  if (x >= 1024 || y >= 1024) return;
  tile_data_[y * kMapExtent + x] = id;
}

void Map::SetTileId(const Vector2f& position, TileId id) {
  u16 x = static_cast<u16>(position.x);
  u16 y = static_cast<u16>(position.y);

  SetTileId(x, y, id);
}

bool Map::IsSolid(TileId id) const {
  if (id == 0) return false;
  if (id >= 162 && id <= 169) return false;  // treat doors as non-solid
  if (id < 170) return true;
  if (id >= 192 && id <= 240) return true;
  if (id >= 242 && id <= 252) return true;

  return false;
}

bool Map::IsSolid(const Vector2f& position) const {
  u16 x = static_cast<u16>(position.x);
  u16 y = static_cast<u16>(position.y);

  return IsSolid(x, y);
}

TileId Map::GetTileId(const Vector2f& position) const {
  u16 x = static_cast<u16>(position.x);
  u16 y = static_cast<u16>(position.y);

  return GetTileId(x, y);
}

bool Map::CanOccupy(const Vector2f& position, float radius) const {

  // casting the result to int always rounds towards 0
  int tile_diameter = (int)((radius + 0.5f) * 2);

  /* if the ship is a 2 x 2 square, assume the upper left square is the position and the radius expands down
  and to the right. This will push the pathline to the upper left for tight corners but also allows bot to path 
  through any size hole the ship will fit through.  Path could be fixed with smoothing. */

  for (uint16_t y = 0; y < tile_diameter; ++y) {
    for (uint16_t x = 0; x < tile_diameter; ++x) {
      uint16_t world_x = (uint16_t)(position.x + x);
      uint16_t world_y = (uint16_t)(position.y + y);
      if (IsSolid(world_x, world_y)) {
        return false;
      }
    }
  }
  return true;
}

struct Tile {
  u32 x : 12;
  u32 y : 12;
  u32 tile : 8;
};

std::unique_ptr<Map> Map::Load(const char* filename) {
  std::ifstream input(filename, std::ios::in | std::ios::binary);

  if (!input.is_open()) return nullptr;

  input.seekg(0, std::ios::end);
  std::size_t size = static_cast<std::size_t>(input.tellg());
  input.seekg(0, std::ios::beg);

  std::string data;
  data.resize(size);

  input.read(&data[0], size);

  std::size_t pos = 0;
  if (data[0] == 'B' && data[1] == 'M') {
    pos = *(u32*)(&data[2]);
  }

  TileData tiles(kMapExtent * kMapExtent);

  while (pos < size) {
    Tile tile = *reinterpret_cast<Tile*>(&data[pos]);

    tiles[tile.y * kMapExtent + tile.x] = tile.tile;

    if (tile.tile == 217) {
      // Mark large asteroid
      for (std::size_t y = 0; y < 2; ++y) {
        for (std::size_t x = 0; x < 2; ++x) {
          tiles[(tile.y + y) * kMapExtent + (tile.x + x)] = tile.tile;
        }
      }
    } else if (tile.tile == 219) {
      // Mark space station tiles
      for (std::size_t y = 0; y < 6; ++y) {
        for (std::size_t x = 0; x < 6; ++x) {
          tiles[(tile.y + y) * kMapExtent + (tile.x + x)] = tile.tile;
        }
      }
    } else if (tile.tile == 220) {
      // Mark wormhole
      for (std::size_t y = 0; y < 5; ++y) {
        for (std::size_t x = 0; x < 5; ++x) {
          tiles[(tile.y + y) * kMapExtent + (tile.x + x)] = tile.tile;
        }
      }
    }

    pos += sizeof(Tile);
  }

  return std::make_unique<Map>(tiles);
}

std::unique_ptr<Map> Map::Load(const std::string& filename) {
  return Load(filename.c_str());
}

}  // namespace marvin
