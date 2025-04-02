#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

#include "../src/ops.h"
#include "../src/player.h"

Vector2 screen{1280, 720};
const float tile_size = 40.0f;
const int cols = screen.x / tile_size;
const int rows = screen.y / tile_size;

std::vector<std::vector<int>> map(rows, std::vector<int>(cols, 0));

bool inbounds(int x, int y) {
  return 0 <= x && x < cols && 0 <= y && y < rows;
}

void saveMap(const char* filename) {
  std::ofstream file(filename);
  if (file.is_open()) {
    for (const auto& row : map) {
      for (int cell : row) {
        file << cell << ' ';
      }
      file << '\n';
    }
    file.close();
  }
}

void loadMap(const char* filename) {
  std::ifstream file(filename);
  if (file.is_open()) {
    for (auto& row : map) {
      for (int& cell : row) {
        file >> cell;
      }
    }
    file.close();
  }
}

void resetMap() {
  for (auto& row : map) {
    std::fill(row.begin(), row.end(), 0);
  }
}

void drawMap() {
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (map[y][x] == 1) {
        DrawRectangle(x * tile_size, y * tile_size, tile_size, tile_size, RED);
      }
    }
  }
}

void drawGrid(float sub = false) {
  for (int x = 0; x < cols; x++) {
    DrawLine(x * tile_size, 0, x * tile_size, screen.y, LIGHTGRAY);
  }
  for (int y = 0; y < rows; y++) {
    DrawLine(0, y * tile_size, screen.x, y * tile_size, LIGHTGRAY);
  }

  if (!sub) return;

  for (int x = 0; x < cols * 2; x++) {
    if (x % 2 == 0) continue;
    DrawLine(x * tile_size / 2, 0, x * tile_size / 2, screen.y, LIGHTGRAY);
  }
  for (int y = 0; y < rows * 2; y++) {
    if (y % 2 == 0) continue;
    DrawLine(0, y * tile_size / 2, screen.x, y * tile_size / 2, LIGHTGRAY);
  }
}

Player player{
  .pos = {0, 0},
  .size = {25, 25},
  .dir = {0, 0},
  .speed = 250,
};

int main() {
  InitWindow(screen.x, screen.y, "Maps");

  printf("cols=%d rows=%d\n", cols, rows);

  while (!WindowShouldClose()) {
    Vector2 mouse_pos = GetMousePosition();
    int x = mouse_pos.x / tile_size;
    int y = mouse_pos.y / tile_size;

    if (inbounds(x, y)) {
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        map[y][x] = 1;
      }
      if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        map[y][x] = 0;
      }
    }

    if (IsKeyPressed(KEY_S)) saveMap("map.txt");
    if (IsKeyPressed(KEY_L)) loadMap("map.txt");
    if (IsKeyPressed(KEY_R)) resetMap();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawGrid();
    drawMap();

    DrawFPS(0, 0);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
