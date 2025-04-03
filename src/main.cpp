#include <raylib.h>

#include <vector>

#include "ops.h"
#include "player.h"

const Color BACKGROUND = GetColor(0x282828ff);
const Color CHECKPOINT = GetColor(0x91eda9ff);
Vector2 screen{1280, 720};
float ball_speed = 300;

enum Screen {
  Start,
  Gameplay,
  Complete,
  GameOver,
};

struct Vec2 : public Vector2 {
  Vec2(float x, float y) : Vector2{x, y} {}

  void norm() {
    float length = sqrtf(x * x + y * y);
    if (length > 0) *this /= length;
  }
};

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
  std::array<Circle, 5> circles;
  int current_checkpoint = -1;

  Level() {
    bounds = {
      .x = screen.x / 2 - 400,
      .y = screen.y / 2 - 250,
      .width = 800,
      .height = 500,
    };

    start = {
      .pos = {bounds.x, bounds.y + bounds.height / 2 - 50},
      .size = 100,
    };

    finish = {
      .pos = {bounds.x + bounds.width - 100, bounds.y + bounds.height / 2 - 50},
      .size = 100,
    };

    checkpoints.push_back(Square{
      .pos = {bounds.x + bounds.width / 2, bounds.y + bounds.height * 0.20f},
      .size = 50,
    });

    checkpoints.push_back(Square{
      .pos = {bounds.x + bounds.width / 2, bounds.y + bounds.height * 0.80f - 25 - 50},
      .size = 50,
    });

    initCircles();
  }

  void initCircles() {
    float length = bounds.width - 200;
    for (size_t i = 0; i < circles.size(); i++) {
      circles[i].pos = {bounds.x + 25 + length / 5 * (i + 1), bounds.y + bounds.height / 2};
      circles[i].radius = 12.5;
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
  Level level;
  Player player = {.size = {25, 25}, .speed = 250};
  Screen current_screen = Start;

  Game() { InitWindow(screen.x, screen.y, "Tiler"); }

  void updateStartScreen() {
    level.setPlayer(player);

    if (IsKeyPressed(KEY_ENTER)) {
      current_screen = Gameplay;
    }
  }

  void updateGameplayScreen() {
    float dt = GetFrameTime();

    player.update(dt);

    for (auto& circle : level.circles) {
      circle.pos.y += ball_speed * dt;

      if (CheckCollisionCircleRec(circle.pos, circle.radius, player.rect())) {
        current_screen = GameOver;
        return;
      }

      if (circle.pos.y + circle.radius >= level.bounds.y + level.bounds.height ||
          circle.pos.y - circle.radius <= level.bounds.y) {
        ball_speed *= -1;
      }
    }

    for (size_t i = 0; i < level.checkpoints.size(); i++) {
      if (CheckCollisionRecs(player.rect(), level.checkpoints[i].rect())) {
        level.current_checkpoint = i;
        break;
      }
    }

    if (CheckCollisionRecs(player.rect(), level.finish.rect())) {
      current_screen = Complete;
      return;
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

    for (const auto& circle : level.circles) {
      DrawCircleV(circle.pos, circle.radius, BLUE);
    }

    DrawRectangleLines(
      level.bounds.x,
      level.bounds.y,
      level.bounds.width,
      level.bounds.height,
      WHITE
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
    ClearBackground(BACKGROUND);
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
