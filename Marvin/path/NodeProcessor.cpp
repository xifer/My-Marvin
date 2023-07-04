#include "NodeProcessor.h"

#include "../Debug.h"

namespace marvin {
namespace path {

NodeConnections NodeProcessor::FindEdges(Node* node, float radius) {
  /* There are some cases where the ship can path into a node diagonally but should not (diagonal gaps)
     when finding edges, only allow diagonal movement if both of the other sides can be pathed on

     note: there are some cases where it would be safe to path diagonally if only one side is pathable,
     this method ignores those cases.
  */
  NodeConnections connections;
  connections.count = 0;

  NodePoint base_point = GetPoint(node);
  MapCoord base(base_point.x, base_point.y);

  bool north = false;
  bool south = false;

  // start by looping through sides first

  Vector2f pos;
  std::vector<Vector2f> neighbors{North(pos), South(pos), West(pos), East(pos)};

  for (std::size_t i = 0; i < neighbors.size(); i++) {
    uint16_t world_x = base_point.x + (uint16_t)neighbors[i].x;
    uint16_t world_y = base_point.y + (uint16_t)neighbors[i].y;
    MapCoord current_pos(world_x, world_y);

    if (!map_.CanOccupyRadius(pos, radius)) {
      if (!map_.CanMoveTo(base, current_pos, radius)) {
        continue;
      }
    }

    NodePoint current_point(world_x, world_y);
    Node* current = GetNode(current_point);

    if (!current) {
      continue;
    }
    //if (!current->is_pathable) {
     // continue;
   // }
    if (map_.IsMined(MapCoord(world_x, world_y))) {
      current->weight = 100.0f;
    } else if (map_.GetTileId(current_point.x, current_point.y) == kSafeTileId) {
      current->weight = 10.0f;
    } else if (current->weight != current->previous_weight) {
      current->weight = current->previous_weight;
    }

    connections.neighbors[connections.count++] = current;

    if (connections.count >= 8) {
      return connections;
    }

    // if a side node gets pushed into the connections, check which node and add diagonal neighbors
    switch (i) {
      case 0: {
        north = true;
      } break;
      case 1: {
        south = true;
      } break;
      case 2: {
        if (north == true) {
          neighbors.push_back(NorthWest(pos));
        }
        if (south == true) {
          neighbors.push_back(SouthWest(pos));
        }
      } break;
      case 3: {
        if (north == true) {
          neighbors.push_back(NorthEast(pos));
        }
        if (south == true) {
          neighbors.push_back(SouthEast(pos));
        }
      } break;
    }
  }
  // }
  return connections;
}

Node* NodeProcessor::GetNode(NodePoint point) {
  if (point.x >= 1024 || point.y >= 1024) {
    return nullptr;
  }

  std::size_t index = point.y * 1024 + point.x;
  Node* node = &nodes_[index];

  if (!(node->flags & NodeFlag_Initialized)) {
    node->g = node->f = 0.0f;
    node->flags = NodeFlag_Initialized;
    node->parent = nullptr;
  }

  return &nodes_[index];
}

}  // namespace path
}  // namespace marvin
