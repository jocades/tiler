#include <raylib.h>

#include <vector>

#include "ops.h"
#include "player.h"

const Color CHECKPOINT = GetColor(0x91eda9ff);

const Vector2 screen{1280, 720};
const float tile_size = 40.0f;
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
  int current_checkpoint = -1;

  Level() {
    bounds = {
      .x = (cols / 2.0f - 8) * tile_size,
      .y = (rows / 2.0f - 6) * tile_size,
      .width = 17 * tile_size,
      .height = 13 * tile_size,
    };

    start = {
      .pos = {bounds.x, bounds.y + bounds.height / 2 - tile_size},
      .size = 2 * tile_size,
    };

    finish = {
      .pos = {bounds.x + bounds.width - 2 * tile_size, bounds.y + bounds.height / 2 - tile_size},
      .size = 2 * tile_size,
    };

    checkpoints.push_back(Square{
      .pos = {bounds.x + 8 * tile_size, bounds.y + 2 * tile_size},
      .size = tile_size,
    });

    checkpoints.push_back(Square{
      .pos = {bounds.x + 8 * tile_size, bounds.y + bounds.height - 3 * tile_size},
      .size = tile_size,
    });

    initCircles();
  }

  void initCircles() {
    int num_tiles = abs(start.pos.x + start.size - finish.pos.x) / tile_size;

    for (int i = 0; i < num_tiles; i++) {
      if (i % 2 == 0) continue;
      circles.push_back(Circle{
        .pos = {(bounds.x + (2 + i) * tile_size + tile_size / 2), bounds.y + bounds.height / 2},
        .radius = 12.5,

      });
    }
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
    initCircles();
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
  Player player = {.size = {25, 25}, .speed = 250};
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

    // for (auto& circle : level.circles) {
    //   circle.pos.y += ball_speed * dt;
    //
    //   if (CheckCollisionCircleRec(circle.pos, circle.radius, player.rect())) {
    //     current_screen = GameOver;
    //     return;
    //   }
    //
    //   if (circle.pos.y + circle.radius >= level.bounds.y + level.bounds.height ||
    //       circle.pos.y - circle.radius <= level.bounds.y) {
    //     ball_speed *= -1;
    //   }
    // }
    //
    // for (size_t i = 0; i < level.checkpoints.size(); i++) {
    //   if (CheckCollisionRecs(player.rect(), level.checkpoints[i].rect())) {
    //     level.current_checkpoint = i;
    //     break;
    //   }
    // }
    //
    // if (CheckCollisionRecs(player.rect(), level.finish.rect())) {
    //   current_screen = Complete;
    //   return;
    // }
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

    for (const auto& circle : level.circles) {
      DrawCircleV(circle.pos, circle.radius, BLUE);
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
