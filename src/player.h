#pragma once

#include <raylib.h>

#include "animation.h"
#include "level.h"
#include "vec2.h"

class Player {
 public:
  vec2 pos;
  vec2 dir;
  vec2 size = {25, 25};
  float speed = 200;
  float dead = false;
  FadeAnimation fade{1, 21};

  Player() = default;

  void input();
  void move(float dt, Level* level);
  void update(float dt, Level* level);
  void draw() const;
  Rectangle rect() const;
};
