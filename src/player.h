#pragma once

#include <raylib.h>

struct Player {
  Vector2 pos = {0, 0};
  Vector2 size;
  Vector2 dir = {0, 0};
  float speed;

  Rectangle rect() const;

  void input();
  void move(float dt);

  void update(float dt);
  void draw() const;
};
