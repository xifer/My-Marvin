
#pragma once

#include <fstream>
#include <vector>
#include <bitset>

#include "Vector2f.h"
#include "MapCoord.h"
#include "platform/Platform.h"

#define DEBUG_RENDER 1
#define DEBUG_USER_CONTROL 0

#define DEBUG_RENDER_GAMEPROXY 0

#define DUBUG_RENDER_PATH 0

#define DEBUG_RENDER_BASE_PATHS 0

#define DEBUG_RENDER_INFLUENCE 0
#define DEBUG_RENDER_INFLUENCE_TEXT 0

#define DEBUG_RENDER_REGION_REGISTRY 0
#define DEBUG_RENDER_PATHFINDER 0
#define DEBUG_RENDER_PATHNODESEARCH 0

#define DEBUG_RENDER_SHOOTER 0
#define DEBUG_RENDER_CALCULATESHOT 0
#define DEBUG_RENDER_BOUNCINGSHOT 0

#define DEBUG_RENDER_FIND_ENEMY_IN_BASE_NODE 0

#define DEBUG_DISABLE_BEHAVIOR 0

namespace marvin {

enum class TextColor { White, Green, Blue, Red, Yellow, Fuchsia, DarkRed, Pink };

enum RenderTextFlags {
  RenderText_Centered = (1 << 1),
};

struct RenderableText {
  std::string text;
  Vector2f at;
  TextColor color;
  int flags;
};

struct RenderableLine {
  Vector2f from;
  Vector2f to;
  COLORREF color;
};

void RenderWorldLine(Vector2f screenCenterWorldPosition, Vector2f from, Vector2f to, COLORREF color);
void RenderDirection(Vector2f screenCenterWorldPosition, Vector2f from, Vector2f direction, float length);
void RenderWorldTile(Vector2f screenCenterWorldPosition, MapCoord position, COLORREF color);
void RenderWorldBox(Vector2f screenCenterWorldPosition, Vector2f position, float size);
void RenderWorldBox(Vector2f screenCenterWorldPosition, Vector2f box_top_left, Vector2f box_bottom_right,
                    COLORREF color);
void RenderWorldText(Vector2f screenCenterWorldPosition, const std::string& text, const Vector2f& at, TextColor color,
                     int flags = 0);
void RenderLine(Vector2f from, Vector2f to, COLORREF color);
// void RenderText(std::string text, Vector2f at, COLORREF color, int flags = 0);
void RenderText(std::string, Vector2f at, TextColor color, int flags = 0);
void RenderPlayerPath(Vector2f position, std::vector<Vector2f> path);
void RenderPath(Vector2f position, std::vector<Vector2f> path);
// void WaitForSync();

Vector2f GetWindowCenter();


// writing to the log file must end with std::endl
class LogFile {
 public:
     ~LogFile() { 
         log.close(); 
     }
  void Open(const std::string& name) {
    if (!log.is_open()) {
      log.open(name + ".log", std::ios::out | std::ios::trunc);
    }
  }
  bool IsOpen() { 
      return log.is_open();
  }
  void Write(const std::string& msg, uint64_t timer) {
    if (log.is_open()) {
      log << msg << " - Time: " << timer << std::endl;
    }
  }
  void Write(const std::string& msg) {
    if (log.is_open()) {
      log << msg << std::endl;
    } 
  }

 private:
  std::ofstream log;
};

//extern std::ofstream log;
extern LogFile log;

struct RenderState {
  static const bool kDisplayDebugText;
  float debug_y;

  std::vector<RenderableText> renderable_texts;
  std::vector<RenderableLine> renderable_lines;

  void Render();

  void RenderDebugText(const char* fmt, ...);
  void RenderDebugText(const std::string& input);

  template <typename T>
  void RenderDebugInBinary(std::string text, T data) {
    std::bitset<sizeof(T) * 8> bits(data);
    RenderText(text + bits.to_string(), Vector2f(GetWindowCenter().x + 150.0f, debug_y), TextColor::Pink, 0);
    debug_y += 12.0f;
  }

  //void RenderDebugInBinary(const std::string& input, unsigned something, int bit_depth);
};

extern RenderState g_RenderState;

}  // namespace marvin