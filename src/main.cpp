#include <raylib.h>

#include <algorithm>
#include <optional>
#include <vector>

#include "ops.h"
#include "player.h"

const Color CHECKPOINT = GetColor(0x91eda9ff);

const Vector2 screen{1280, 720};
const float tile_size = 40.0f;
const float half_tile = tile_size / 2;
const int cols = screen.x / tile_size;
const int rows = screen.y / tile_size;

float ball_speed = 300;

void drawGrid() {
  for (int x = 0; x < cols; x++) {
    DrawLine(x * tile_size, 0, x * tile_size, screen.y, LIGHTGRAY);
  }
  for (int y = 0; y < rows; y++) {
    DrawLine(0, y * tile_size, screen.x, y * tile_size, LIGHTGRAY);
  }
}

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

struct Level {
  Rectangle bounds;
  Square start;
  Square finish;
  std::vector<Square> checkpoints;
  std::vector<Circle> circles;
  std::vector<Circle> coins;
  std::vector<Obstacle> obstacles;
  int current_checkpoint = -1;

  Level() {
    bounds = {
      .x = (cols / 2.0f - 8) * tile_size,
      .y = (rows / 2.0f - 4) * tile_size,
      .width = 16 * tile_size,
      .height = 8 * tile_size,
    };

    start = {
      .pos = {bounds.x, bounds.y + bounds.height / 2 - tile_size},
      .size = 2 * tile_size,
    };

    finish = {
      .pos = {bounds.x + bounds.width - 2 * tile_size, bounds.y + bounds.height / 2 - tile_size},
      .size = 2 * tile_size,
    };

    // checkpoints.push_back(Square{
    //   .pos = {bounds.x + 8 * tile_size, bounds.y + 2 * tile_size},
    //   .size = tile_size,
    // });
    //
    // checkpoints.push_back(Square{
    //   .pos = {bounds.x + 8 * tile_size, bounds.y + bounds.height - 3 * tile_size},
    //   .size = tile_size,
    // });

    initObstacles();
    initCoins();
  }

  void initObstacles() {
    int num_tiles = abs(start.pos.x + start.size - finish.pos.x) / tile_size;
    for (int i = 0; i < num_tiles; i++) {
      float x = (bounds.x + (2 + i) * tile_size) + half_tile;
      if (i % 2 == 0) {
        obstacles.emplace_back(Vector2{x, bounds.y + half_tile + 20}, 12.5, Vector2{0, 1}, 200);
      } else {
        obstacles.emplace_back(
          Vector2{x, bounds.y + bounds.height - half_tile - 20},
          12.5,
          Vector2{0, -1},
          200
        );
      }
    }
  }

  void initCircles() {
    int num_tiles = abs(start.pos.x + start.size - finish.pos.x) / tile_size;

    for (int i = 0; i < num_tiles; i++) {
      if (i % 2 == 0) {
        circles.push_back(Circle{
          .pos = {(bounds.x + (2 + i) * tile_size + tile_size / 2), bounds.y + bounds.height / 2},
          .radius = 12.5,
        });
      } else {
      }
    }
  }

  void initCoins() {
    coins.push_back(Circle{
      .pos = {bounds.x + 8 * tile_size, bounds.y + 4 * tile_size},
      .radius = 10,
    });
  }

  void setPlayer(Player& player) {
    if (current_checkpoint != -1) {
      const auto& checkpoint = checkpoints[current_checkpoint];
      player.pos = checkpoint.pos + checkpoint.size / 2 - player.size / 2;
    } else {
      player.pos = start.pos + start.size / 2 - player.size / 2;
    }
  }

  void reset(Player& player) {
    setPlayer(player);
    obstacles.clear();
    initObstacles();
    coins.clear();
    initCoins();
  }
};

struct Game {
  enum Screen {
    Start,
    Gameplay,
    Complete,
    GameOver,
  };

  Level level;
  Player player = {.size = {25, 25}, .speed = 200};
  Screen current_screen = Start;

  Game() {
    SetTraceLogLevel(LOG_TRACE);
    TraceLog(LOG_TRACE, "cols=%d rows=%d", cols, rows);
    InitWindow(screen.x, screen.y, "Tiler");
  }

  void updateStartScreen() {
    level.setPlayer(player);

    if (IsKeyPressed(KEY_ENTER)) {
      current_screen = Gameplay;
    }
  }

  void updateGameplayScreen() {
    float dt = GetFrameTime();

    player.update(dt);
    auto player_rect = player.rect();

    for (auto& obs : level.obstacles) {
      if (CheckCollisionCircleRec(obs.pos, obs.radius, player_rect)) {
        current_screen = GameOver;
        return;
      }

      if (obs.pos.y <= level.bounds.y + half_tile ||
          obs.pos.y >= level.bounds.y + level.bounds.height - half_tile) {
        obs.dir.y = -obs.dir.y;
      }

      obs.pos += obs.dir * obs.speed * dt;
    }

    for (size_t i = 0; i < level.checkpoints.size(); i++) {
      if (CheckCollisionRecs(player_rect, level.checkpoints[i].rect())) {
        level.current_checkpoint = i;
        break;
      }
    }

    std::erase_if(level.coins, [&player_rect](const Circle& coin) {
      return CheckCollisionCircleRec(coin.pos, coin.radius, player_rect);
    });

    if (level.coins.empty() && CheckCollisionRecs(player_rect, level.finish.rect())) {
      current_screen = Complete;
    }
  }

  void updateCompleteScreen() {
    if (IsKeyPressed(KEY_ENTER)) {
      level.current_checkpoint = -1;
      level.reset(player);
      current_screen = Gameplay;
    }
  }

  void updateGameOverScreen() {
    if (IsKeyPressed(KEY_ENTER)) {
      level.reset(player);
      current_screen = Gameplay;
    }
  }

  void update() {
    switch (current_screen) {
      case Start: updateStartScreen(); break;
      case Gameplay: updateGameplayScreen(); break;
      case Complete: updateCompleteScreen(); break;
      case GameOver: updateGameOverScreen(); break;
    }
  }

  void drawStartScreen() {
    const char* text = "START SCREEN";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(
      text,
      screen.x / 2 - text_width / 2,
      screen.y / 2 - font_size / 2,
      font_size,
      LIGHTGRAY
    );
  }

  void drawGameplayScreen() {
    DrawRectangleRec(level.start.rect(), LIGHTGRAY);
    DrawRectangleRec(level.finish.rect(), LIGHTGRAY);

    for (auto& checkpoint : level.checkpoints) {
      DrawRectangleRec(checkpoint.rect(), CHECKPOINT);
    }

    player.draw();

    // for (const auto& circle : level.circles) {
    //   DrawCircleV(circle.pos, circle.radius, BLUE);
    // }

    for (const auto& obs : level.obstacles) {
      DrawCircleV(obs.pos, obs.radius, BLUE);
    }

    for (const auto& coin : level.coins) {
      DrawCircleV(coin.pos, coin.radius, YELLOW);
      DrawCircleLinesV(coin.pos, coin.radius, BLACK);
    }

    DrawRectangleLines(
      level.bounds.x,
      level.bounds.y,
      level.bounds.width,
      level.bounds.height,
      BLACK
    );
  }

  void drawCompleteScreen() {
    const char* text = "LEVEL COMPLETE";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(
      text,
      screen.x / 2 - text_width / 2,
      screen.y / 2 - font_size / 2,
      font_size,
      LIGHTGRAY
    );
  }

  void drawGameOverScreen() {
    const char* text = "GAME OVER";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(
      text,
      screen.x / 2 - text_width / 2,
      screen.y / 2 - font_size / 2,
      font_size,
      LIGHTGRAY
    );
  }

  void draw() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    drawGrid();
    switch (current_screen) {
      case Start: drawStartScreen(); break;
      case Gameplay: drawGameplayScreen(); break;
      case Complete: drawCompleteScreen(); break;
      case GameOver: drawGameOverScreen(); break;
    }
    DrawFPS(0, 0);
    EndDrawing();
  }

  void run() {
    while (!WindowShouldClose()) {
      update();
      draw();
    }
  }

  ~Game() { CloseWindow(); }
};

int main() {
  Game game;
  game.run();
}
