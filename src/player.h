#pragma once

#include <raylib.h>

struct Player {
  Vector2 pos;
  Vector2 size;
  Vector2 dir;
  float speed;

  Rectangle rect() const;

  void input();
  void move(float dt);

  void update(float dt);
  void draw() const;
};
