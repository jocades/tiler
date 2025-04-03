#include <raylib.h>

#include <cmath>
#include <cstddef>
#include <string>

Vector2 screen{800, 400};

const float tile_size = 40.0f;
const int cols = screen.x / tile_size;
const int rows = screen.y / tile_size;

std::vector<std::string> level = {"1111111111"};

// int level[map_height][map_width] = {
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
// };
//
// int level2[map_height][map_width] = {
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
//   {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
//   {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
//   {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
//   {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
//   {1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
// };

void drawAxis() {
  DrawLineV({screen.x / 2, 0}, {screen.x / 2, screen.y}, BLACK);
  DrawLineV({0, screen.y / 2}, {screen.x, screen.y / 2}, BLACK);
}

// void drawMap() {
//   float offset_x = (screen.x - (map_width * tile_size)) / 2;
//   float offset_y = (screen.y - (map_height * tile_size)) / 2;
//
//   for (int y = 0; y < map_height; y++) {
//     for (int x = 0; x < map_width; x++) {
//       Vector2 pos{
//         offset_x + x * tile_size,
//         offset_y + y * tile_size,
//       };
//
//       // if (level[y][x] == 1) {
//       //   DrawRectangleV(pos, {tile_size, tile_size}, RED);
//       // }
//       if (level2[y][x] == 1) {
//         DrawRectangleV(pos, {tile_size, tile_size}, (x + y) % 2 == 0 ? DARKGRAY : LIGHTGRAY);
//       }
//     }
//   }
// }

int main() {
  InitWindow(screen.x, screen.y, "Maps");

  while (!WindowShouldClose()) {
    Vector2 mouse_pos = GetMousePosition();

    BeginDrawing();
    ClearBackground(RAYWHITE);

    for (size_t x = 0; x < screen.x / tile_size; x++) {
      DrawLine(x * tile_size, 0, x * tile_size, screen.y, LIGHTGRAY);
    }

    for (size_t y = 0; y < screen.y / tile_size; y++) {
      DrawLine(0, y * tile_size, screen.x, y * tile_size, LIGHTGRAY);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
