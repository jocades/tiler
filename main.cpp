#include <raylib.h>

#include "ops.h"

const Color BACKGROUND = GetColor(0x282828ff);
Vector2 screen{1280, 720};

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

Player player{
  .pos = screen / 3,
  .size = {25, 25},
  .dir = {0, 0},
  .speed = 500,
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

struct Vec2 {
  float x;
  float y;
};

int main() {
  InitWindow(screen.x, screen.y, "Tiler");

  Rectangle level{
    .x = screen.x / 2 - 400,
    .y = screen.y / 2 - 250,
    .width = 800,
    .height = 500,
  };

  Square start{
    .pos = {level.x + 5, level.y + level.height / 2 - 50},
    .size = 100,
  };

  Square finish{
    .pos = {level.x + level.width - 100, level.y + level.height / 2 - 50},
    .size = 100,
  };

  player.pos.x = start.pos.x + start.size / 2;
  player.pos.y = start.pos.y + start.size / 2;

  Circle circle{
    .pos = {level.x + level.width / 2, level.y + 5},
    .radius = 25.0f / 2,
  };

  float ball_speed = 250;

  float length = level.width - 200;

  std::array<Circle, 5> circles;
  for (size_t i = 0; i < circles.size(); i++) {
    circles[i].pos = {level.x + length / 5 * (i + 1), level.y};
    circles[i].radius = 12.5;
  }

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    circle.pos.y += ball_speed * dt;
    if (circle.pos.y + circle.radius >= level.y + level.height ||
        circle.pos.y - circle.radius <= level.y) {
      ball_speed *= -1;
    }

    player.dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    player.dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
    normalize(player.dir);

    player.pos += player.dir * player.speed * dt;

    bool collision = CheckCollisionCircleRec(circle.pos, circle.radius, player.rect());

    if (collision) {
      // break;
    }

    BeginDrawing();
    ClearBackground(BACKGROUND);

    DrawRectangleRec(start.rect(), LIGHTGRAY);
    DrawRectangleRec(finish.rect(), LIGHTGRAY);

    DrawRectangleV(player.pos, player.size, ORANGE);
    DrawCircleV(circle.pos, circle.radius, BLUE);

    for (const auto& circle : circles) {
      DrawCircleV(circle.pos, circle.radius, BLUE);
    }

    DrawRectangleLinesEx(level, 5, WHITE);
    // DrawRectangleLines(level.x, level.y, level.width, level.height, WHITE);

    if (collision) {
      DrawCircleLinesV(circle.pos, circle.radius, RED);
    }

    DrawFPS(0, 0);
    EndDrawing();
  }

  CloseWindow();
}
