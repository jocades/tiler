#include <raylib.h>

#include <string>

#include "../src/ops.h"

Vector2 screen{1280, 720};

const float size = 40.0f;

std::vector<std::string> level = {
  "1111111111",
  "1000000001",
  "1000000001",
  "1000000001",
  "1000000001",
  "1000000001",
  "1000000001",
  "1000000001",
  "1000000001",
  "1111111111",
};

int rows = level.size();
int cols = level[0].size();

void drawGrid() {
  for (int x = 0; x < screen.x / size; x++) {
    DrawLine(x * size, 0, x * size, screen.y, LIGHTGRAY);
  }
  for (int y = 0; y < screen.y / 2; y++) {
    DrawLine(0, y * size, screen.x, y * size, LIGHTGRAY);
  }
}

bool isWall(int x, int y) {
  if (!(0 <= x && x < cols && 0 <= y && y < rows)) return false;
  return level[y][x] == '1';
}

int start_x = (cols / 2.0 + 6) * size;
int start_y = (rows / 2.0f - 1) * size;

std::vector<std::pair<Vector2, Vector2>> perimeter;

void calculatePerimeter() {
  perimeter.clear();  // Reset previous perimeter

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (level[y][x] != '0') continue;  // Skip non-floor tiles

      float worldX = start_x + x * size;
      float worldY = start_y + y * size;

      // Check if the floor tile is at the edge or adjacent to a wall
      bool isEdge = (x == 0 || x == cols - 1 || y == 0 || y == rows - 1);  // Edge of the map
      bool adjacentToWall =
        isWall(x - 1, y) || isWall(x + 1, y) || isWall(x, y - 1) || isWall(x, y + 1);

      // If the tile is at the edge of the map or adjacent to a wall, it's part of the perimeter
      if (isEdge || adjacentToWall) {
        // Top edge of the floor tile
        if (y == 0 || level[y - 1][x] == '1') {
          perimeter.push_back({{worldX, worldY}, {worldX + size, worldY}});
        }
        // Bottom edge of the floor tile
        if (y == rows - 1 || level[y + 1][x] == '1') {
          perimeter.push_back({{worldX, worldY + size}, {worldX + size, worldY + size}});
        }
        // Left edge of the floor tile
        if (x == 0 || level[y][x - 1] == '1') {
          perimeter.push_back({{worldX, worldY}, {worldX, worldY + size}});
        }
        // Right edge of the floor tile
        if (x == cols - 1 || level[y][x + 1] == '1') {
          perimeter.push_back({{worldX + size, worldY}, {worldX + size, worldY + size}});
        }
      }
    }
  }
}

void drawPerimeter() {
  for (const auto& edge : perimeter) {
    DrawLineV(edge.first, edge.second, BLACK);
  }
}

int main() {
  InitWindow(screen.x, screen.y, "Maps");

  calculatePerimeter();

  Rectangle player{
    start_x + 5 * size,
    start_y + 5 * size,
    20,
    20,
  };

  float speed = 400;

  printf("rows=%d cols=%d\n", rows, cols);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    Vector2 dir{
      float(IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT)),
      float(IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP)),
    };
    normalize(dir);

    int gx = (player.x - start_x) / size;
    int gy = (player.y - start_y) / size;

    player.x += dir.x * speed * dt;
    int ngx = (player.x + (dir.x > 0 ? player.width : 0) - start_x) / size;
    if (isWall(ngx, gy)) {
      if (dir.x > 0) {
        player.x = (ngx * size) + start_x - player.width;
      }
      if (dir.x < 0) {
        player.x = (ngx + 1) * size + start_x;
      }
    }

    player.y += dir.y * speed * dt;
    int ngy = (player.y + (dir.y > 0 ? player.height : 0) - start_y) / size;
    if (isWall(gx, ngy)) {
      if (dir.y > 0) {
        player.y = (ngy * size) + start_y - player.height;
      }
      if (dir.y < 0) {
        player.y = (ngy + 1) * size + start_y;
      }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawGrid();

    for (int y = 0; y < rows; y++) {
      for (int x = 0; x < cols; x++) {
        if (level[y][x] == '1') continue;
        int world_x = start_x + x * size;
        int world_y = start_y + y * size;
        Color color = (y + x) % 2 == 0 ? DARKGRAY : GRAY;
        DrawRectangle(world_x, world_y, size, size, color);
      }
    }

    drawPerimeter();

    DrawRectangleRec(player, ORANGE);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
