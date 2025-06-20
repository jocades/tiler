#pragma once

#include <raylib.h>

#include <vector>

struct Obstacle {
  Vector2 pos;
  Vector2 dir;
  float radius;
  float speed;
};

struct Coin {
  Vector2 pos;
  float radius;
};

struct Level {
  std::vector<std::vector<int>> map;
  std::vector<std::pair<Vector2, Vector2>> perimeter;
  std::vector<Rectangle> checkpoints;
  std::vector<Obstacle> obstacles;
  std::vector<Coin> coins;

  Level() = default;
  Level(const char* filename);

  void load(const char* filename);
  void draw();

  bool isWall(int x, int y);
};
