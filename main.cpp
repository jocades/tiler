#include <raylib.h>

#include <vector>

#include "ops.h"

const Color BACKGROUND = GetColor(0x282828ff);
const Color CHECKPOINT = GetColor(0x91eda9ff);
Vector2 screen{1280, 720};

enum GameScreen {
  Start,
  Gameplay,
  Complete,
  GameOver,
};

struct Player {
  Vector2 pos;
  Vector2 size;
  Vector2 dir;
  float speed;

  Rectangle rect() const {
    return {
      .x = pos.x,
      .y = pos.y,
      .width = size.x,
      .height = size.y,
    };
  }
};

struct vec2 : public Vector2 {
  vec2(float x, float y) : Vector2{x, y} {}

  void norm() {
    float length = sqrtf(x * x + y * y);
    if (length > 0) *this /= length;
  }
};

void normalize(Vector2& v) {
  float length = sqrtf((v.x * v.x) + (v.y * v.y));
  if (length > 0) v /= length;
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

struct Map {
  Rectangle bounds;
  Square start;
  Square finish;
  std::vector<Square> checkpoints;
  std::array<Circle, 5> circles;
  int current_checkpoint = -1;

  Map() {
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

int main() {
  InitWindow(screen.x, screen.y, "Tiler");

  GameScreen current_screen = Start;

  Player player{
    .pos = {0, 0},
    .size = {25, 25},
    .dir = {0, 0},
    .speed = 250,
  };

  Map map{};

  map.setPlayer(player);

  float ball_speed = 300;

  while (!WindowShouldClose()) {
    switch (current_screen) {
      case Start: {
        if (IsKeyPressed(KEY_ENTER)) {
          current_screen = Gameplay;
        }
      } break;

      case Gameplay: {
        float dt = GetFrameTime();

        player.dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
        player.dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
        normalize(player.dir);

        player.pos += player.dir * player.speed * dt;

        for (auto& circle : map.circles) {
          circle.pos.y += ball_speed * dt;

          if (CheckCollisionCircleRec(circle.pos, circle.radius, player.rect())) {
            current_screen = GameOver;
          }

          if (circle.pos.y + circle.radius >= map.bounds.y + map.bounds.height ||
              circle.pos.y - circle.radius <= map.bounds.y) {
            ball_speed *= -1;
          }
        }

        for (size_t i = 0; i < map.checkpoints.size(); i++) {
          if (CheckCollisionRecs(player.rect(), map.checkpoints[i].rect())) {
            map.current_checkpoint = i;
            break;
          }
        }

        if (CheckCollisionRecs(player.rect(), map.finish.rect())) {
          current_screen = Complete;
        }

      } break;

      case Complete: {
        if (IsKeyPressed(KEY_ENTER)) {
          map.current_checkpoint = -1;
          map.reset(player);
          current_screen = Gameplay;
        }
      } break;

      case GameOver: {
        if (IsKeyPressed(KEY_ENTER)) {
          map.reset(player);
          current_screen = Gameplay;
        }
      } break;
    }

    BeginDrawing();
    ClearBackground(BACKGROUND);

    switch (current_screen) {
      case Start: {
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
      } break;

      case Gameplay: {
        DrawRectangleRec(map.start.rect(), LIGHTGRAY);
        DrawRectangleRec(map.finish.rect(), LIGHTGRAY);

        for (auto& checkpoint : map.checkpoints) {
          DrawRectangleRec(checkpoint.rect(), CHECKPOINT);
        }

        DrawRectangleV(player.pos, player.size, ORANGE);

        for (const auto& circle : map.circles) {
          DrawCircleV(circle.pos, circle.radius, BLUE);
        }

        // DrawRectangleLinesEx(level, 5, WHITE);
        DrawRectangleLines(map.bounds.x, map.bounds.y, map.bounds.width, map.bounds.height, WHITE);

      } break;

      case Complete: {
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
      } break;

      case GameOver: {
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
      } break;
    }

    DrawFPS(0, 0);
    EndDrawing();
  }

  CloseWindow();
}
