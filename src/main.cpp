#include <fstream>

#include "raylib.h"
#include "vec2.h"

const vec2 win = {1280, 720};
const int SIZE = 40;
const int COLS = win.x / SIZE;
const int ROWS = win.y / SIZE;

class Level {
 private:
  std::vector<std::vector<int>> map;
  const std::pair<Color, Color> TILE_COLORS = {GetColor(0xe3e3e3ff), GetColor(0xc7c7c7ff)};

 public:
  Level(const std::string& path) : map(ROWS, std::vector<int>(COLS, 0)) {
    std::ifstream file(path);
    if (file.is_open()) {
      std::string line;
      for (size_t y = 0; std::getline(file, line); y++) {
        for (size_t x = 0; x < line.size(); x++) {
          map[y][x] = line[x] - '0';
        }
      }
      file.close();
    }
  };

  int get(int row, int col) const {
    return map[row][col];
  }

  void draw() {
    for (size_t y = 0; y < map.size(); y++) {
      for (size_t x = 0; x < map[0].size(); x++) {
        if (map[y][x] == 1) {
          DrawRectangle(
            x * SIZE,
            y * SIZE,
            SIZE,
            SIZE,
            (x + y) % 2 == 0 ? TILE_COLORS.first : TILE_COLORS.second
          );
        }
      }
    }
  }
};

bool sweepAABB(vec2& pos, vec2 size, vec2 delta, std::shared_ptr<Level>& level) {
  if (delta.x != 0) {
    float nx = pos.x + delta.x;
    float x_edge = delta.x > 0 ? nx + size.x - 1 : nx;
    int c = x_edge / SIZE;

    int row_start = pos.y / SIZE;
    int row_end = (pos.y + size.y - 1) / SIZE;

    for (int r = row_start; r <= row_end; r++) {
      if (level->get(r, c) == 0) {
        // Snap to edge of tile
        if (delta.x > 0) pos.x = c * SIZE - size.x;
        else pos.x = (c + 1) * SIZE;
        delta.x = 0;
        break;
      }
    }

    pos.x += delta.x;
  }

  if (delta.y != 0) {
    float ny = pos.y + delta.y;
    float y_edge = delta.y > 0 ? ny + size.y - 1 : ny;
    int r = y_edge / SIZE;

    int col_start = pos.x / SIZE;
    int col_end = (pos.x + size.x - 1) / SIZE;

    for (int c = col_start; c <= col_end; c++) {
      if (level->get(r, c) == 0) {
        if (delta.y > 0) pos.y = r * SIZE - size.y;
        else pos.y = (r + 1) * SIZE;
        delta.y = 0;
        break;
      }
    }

    pos.y += delta.y;
  }

  return true;
}

class Player {
 public:
  vec2 pos{win.x / 2, win.y / 2};
  vec2 dir;
  vec2 size = {25, 25};
  float speed = 200;

  Player() = default;

  void input() {
    dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
    dir.norm();
  }

  void move(float dt, std::shared_ptr<Level> level) {
    vec2 delta = dir * speed * dt;
    sweepAABB(pos, size, delta, level);
  }

  void update(float dt, std::shared_ptr<Level> level) {
    input();
    move(dt, level);
  }

  void draw() const {
    DrawRectangleV(pos, size, ORANGE);
  }
};

class Game {
 private:
  enum Screen {
    Start,
    Play,
    Done,
  };

  const Color BG_COLOR = GetColor(0x67a0bfff);
  const Color GRID_COLOR = GetColor(0xbcc2beff);

  Player player;
  std::shared_ptr<Level> level;

 public:
  Game() {
    InitWindow(win.x, win.y, "The Impossible Game");
    level = std::make_shared<Level>("levels/1/map.txt");
  }

  void run() {
    while (!WindowShouldClose()) {
      update();
      draw();
    }
  }

  void update() {
    float dt = GetFrameTime();

    player.update(dt, level);
  }

  void drawGrid(float spacing) {
    for (int x = 0; x < COLS; x++) {
      DrawLine(x * spacing, 0, x * spacing, win.y, GRID_COLOR);
    }
    for (int y = 0; y < ROWS; y++) {
      DrawLine(0, y * spacing, win.x, y * spacing, GRID_COLOR);
    }
  }

  void draw() {
    BeginDrawing();
    ClearBackground(BG_COLOR);

    level->draw();
    drawGrid(SIZE);
    player.draw();

    DrawFPS(0, 0);
    EndDrawing();
  }

  ~Game() {
    CloseWindow();
  }
};

const Color CHECKPOINT = GetColor(0x91eda9ff);

const Vector2 screen{1280, 720};
const float tile_size = 40.0f;
const float half_tile = tile_size / 2;
const int cols = screen.x / tile_size;
const int rows = screen.y / tile_size;

float ball_speed = 300;

struct Circle {
  Vector2 pos;
  float radius;
};

struct Obstacle : public Circle {
  Vector2 dir;
  float speed;

  Obstacle(Vector2 pos, float radius) : Circle{pos, radius} {}
  Obstacle(Vector2 pos, float radius, Vector2 dir, float speed)
      : Circle{pos, radius}, dir(dir), speed(speed) {}
};

struct Square {
  Vector2 pos;
  float size;

  Rectangle rect() const {
    return {
      .x = pos.x,
      .y = pos.y,
      .width = size,
      .height = size,
    };
  }
};

// struct Game {
//   enum Screen {
//     Start,
//     Gameplay,
//     Complete,
//     GameOver,
//   };
//
//   Level level;
//   Player player = {.size = {25, 25}, .speed = 200};
//   Screen current_screen = Start;
//
//   Game() {
//     SetTraceLogLevel(LOG_TRACE);
//     TraceLog(LOG_TRACE, "cols=%d rows=%d", cols, rows);
//     InitWindow(screen.x, screen.y, "Tiler");
//     SetTargetFPS(144);
//   }
//
//   void updateStartScreen() {
//     level.setPlayer(player);
//
//     if (IsKeyPressed(KEY_ENTER)) {
//       current_screen = Gameplay;
//     }
//   }
//
//   void updateGameplayScreen() {
//     float dt = GetFrameTime();
//
//     player.update(dt);
//     auto rect = player.rect();
//
//     // Vector2 sides[4] = {
//     //   {rect.x + rect.width / 2, rect.y},                // top
//     //   {rect.x + rect.width, rect.y + rect.height / 2},  // right
//     //   {rect.x + rect.width / 2, rect.y + rect.height},  // bottom
//     //   {rect.x, rect.y + rect.height / 2},               // left
//     // };
//     //
//     // for (size_t i = 0; i < level.perimeter.size() - 1; i++) {
//     //   for (const auto& corner : sides) {
//     //     if (CheckCollisionPointLine(corner, level.perimeter[i], level.perimeter[i + 1], 2)) {
//     //       if (player.dir.x == 1) {
//     //         player.pos.x = level.perimeter[i].x - player.size.x;
//     //       }
//     //       if (player.dir.x == -1) {
//     //         player.pos.x = level.perimeter[i].x;
//     //       }
//     //       if (player.dir.y == 1) {
//     //         player.pos.y = level.perimeter[i].y - player.size.y;
//     //       }
//     //       if (player.dir.y == -1) {
//     //         player.pos.y = level.perimeter[i].y;
//     //       }
//     //     }
//     //   }
//     // }
//
//     if (player.pos.x <= level.bounds.x) {
//       player.pos.x = level.bounds.x;
//     }
//     if (player.pos.x >= level.bounds.x + level.bounds.width - player.size.x) {
//       player.pos.x = level.bounds.x + level.bounds.width - player.size.x;
//     }
//     if (player.pos.y <= level.bounds.y) {
//       player.pos.y = level.bounds.y;
//     }
//     if (player.pos.y >= level.bounds.y + level.bounds.height - player.size.y) {
//       player.pos.y = level.bounds.y + level.bounds.height - player.size.y;
//     }
//
//     for (auto& obs : level.obstacles) {
//       if (CheckCollisionCircleRec(obs.pos, obs.radius, rect)) {
//         current_screen = GameOver;
//         return;
//       }
//
//       if (obs.pos.y <= level.bounds.y + half_tile ||
//           obs.pos.y >= level.bounds.y + level.bounds.height - half_tile) {
//         obs.dir.y = -obs.dir.y;
//       }
//
//       obs.pos += obs.dir * obs.speed * dt;
//     }
//
//     for (size_t i = 0; i < level.checkpoints.size(); i++) {
//       if (CheckCollisionRecs(rect, level.checkpoints[i].rect())) {
//         level.current_checkpoint = i;
//         break;
//       }
//     }
//
//     std::erase_if(level.coins, [&rect](const Circle& coin) {
//       return CheckCollisionCircleRec(coin.pos, coin.radius, rect);
//     });
//
//     if (level.coins.empty() && CheckCollisionRecs(rect, level.finish.rect())) {
//       current_screen = Complete;
//     }
//   }
//
//   void updateCompleteScreen() {
//     if (IsKeyPressed(KEY_ENTER)) {
//       level.current_checkpoint = -1;
//       level.reset(player);
//       current_screen = Gameplay;
//     }
//   }
//
//   void updateGameOverScreen() {
//     if (IsKeyPressed(KEY_ENTER)) {
//       level.reset(player);
//       current_screen = Gameplay;
//     }
//   }
//
//   void update() {
//     switch (current_screen) {
//       case Start: updateStartScreen(); break;
//       case Gameplay: updateGameplayScreen(); break;
//       case Complete: updateCompleteScreen(); break;
//       case GameOver: updateGameOverScreen(); break;
//     }
//   }
//
//   void drawStartScreen() {
//     const char* text = "START SCREEN";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void drawGameplayScreen() {
//     DrawRectangleRec(level.start.rect(), LIGHTGRAY);
//     DrawRectangleRec(level.finish.rect(), LIGHTGRAY);
//
//     for (auto& checkpoint : level.checkpoints) {
//       DrawRectangleRec(checkpoint.rect(), CHECKPOINT);
//     }
//
//     player.draw();
//
//     for (const auto& obs : level.obstacles) {
//       DrawCircleV(obs.pos, obs.radius, BLUE);
//     }
//
//     for (const auto& coin : level.coins) {
//       DrawCircleV(coin.pos, coin.radius, YELLOW);
//       DrawCircleLinesV(coin.pos, coin.radius, BLACK);
//     }
//
//     for (size_t i = 0; i < level.perimeter.size() - 1; i++) {
//       DrawLineEx(level.perimeter[i], level.perimeter[i + 1], 4, BLACK);
//     }
//   }
//
//   void drawCompleteScreen() {
//     const char* text = "LEVEL COMPLETE";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void drawGameOverScreen() {
//     const char* text = "GAME OVER";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void draw() {
//     BeginDrawing();
//     ClearBackground(RAYWHITE);
//     drawGrid();
//     switch (current_screen) {
//       case Start: drawStartScreen(); break;
//       case Gameplay: drawGameplayScreen(); break;
//       case Complete: drawCompleteScreen(); break;
//       case GameOver: drawGameOverScreen(); break;
//     }
//     DrawFPS(0, 0);
//     EndDrawing();
//   }
//
//   void run() {
//     while (!WindowShouldClose()) {
//       update();
//       draw();
//     }
//   }
//
//   ~Game() {
//     CloseWindow();
//   }
// };

int main() {
  Game game;
  game.run();
}
