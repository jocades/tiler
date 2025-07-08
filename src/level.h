#pragma once

#include <raylib.h>

#include <vector>

#include "vec2.h"

class Move {
 public:
  enum Kind {
    Linear
  };
  Kind kind;
  virtual void update(float dt, vec2& pos) = 0;
  virtual ~Move() = default;
};

struct Bounds {
  vec2 min;
  vec2 max;
};

class Linear : public Move {
 public:
  vec2 dir;
  float speed;
  Bounds bounds;

  Linear(vec2 dir, float speed, Bounds bounds);

  void update(float dt, vec2& pos) override;
};

struct Circle {
  vec2 pos;
  float radius = 10;
  std::unique_ptr<Move> move;

  void update(float dt);
  void draw() const;
};

struct Coin {
  vec2 pos;
  float radius = 7.5;

  Coin(vec2 pos);

  void draw() const;
};

class Level {
 public:
  std::vector<std::vector<char>> map;
  Rectangle start;
  Rectangle finish;
  std::vector<Circle> obstacles;
  std::vector<Rectangle> checkpoints;
  std::vector<Coin> coins;
  int current_checkpoint = -1;

  Level(int id);

  char get(int row, int col) const;
  void set_player(vec2& pos, vec2 size);
  void update(float dt);
  void draw() const;
};

class LevelManager {
 public:
  std::vector<Level> levels;
  size_t index = 0;

  LevelManager(int level_count);

  Level* current();
  Level* next();
};
