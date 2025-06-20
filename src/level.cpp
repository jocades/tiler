#include "level.h"

#include <fstream>

static const float tile_size = 40.0f;

static void createPerimeter(Level& level) {
  int rows = level.map.size();
  int cols = level.map[0].size();

  level.perimeter.clear();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (level.map[y][x] == 0) continue;

      float wx = x * tile_size;
      float wy = y * tile_size;

      bool is_edge = (x == 0 || x == cols - 1 || y == 0 || y == rows - 1);
      bool adj = level.isWall(x - 1, y) || level.isWall(x + 1, y) || level.isWall(x, y - 1) ||
                 level.isWall(x, y + 1);

      if (is_edge || adj) {
        if (y == 0 || level.map[y - 1][x] == 0) {
          level.perimeter.push_back({{wx, wy}, {wx + tile_size, wy}});
        }
        if (y == rows - 1 || level.map[y + 1][x] == 0) {
          level.perimeter.push_back({{wx, wy + tile_size}, {wx + tile_size, wy + tile_size}});
        }
        if (x == 0 || level.map[y][x - 1] == 0) {
          level.perimeter.push_back({{wx, wy}, {wx, wy + tile_size}});
        }
        if (x == cols - 1 || level.map[y][x + 1] == 0) {
          level.perimeter.push_back({{wx + tile_size, wy}, {wx + tile_size, wy + tile_size}});
        }
      }
    }
  }
}

static void createCheckpoints(Level& level) {
  int rows = level.map.size();
  int cols = level.map[0].size();

  std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (level.map[y][x] == 2 && !visited[y][x]) {
        int start_x = x, start_y = y;
        int end_x = x, end_y = y;

        while (end_x + 1 < cols && level.map[y][end_x + 1] == 2) {
          end_x++;
        }

        bool valid = true;
        while (valid && end_y + 1 < rows) {
          for (int i = start_x; i <= end_x; i++) {
            if (level.map[end_y + 1][i] != 2) {
              valid = false;
              break;
            }
          }
          if (valid) end_y++;
        }

        for (int i = start_y; i <= end_y; i++) {
          for (int j = start_x; j <= end_x; j++) {
            visited[i][j] = true;
          }
        }

        level.checkpoints.push_back({
          start_x * tile_size,
          start_y * tile_size,
          (end_x - start_x + 1) * tile_size,
          (end_y - start_y + 1) * tile_size,
        });
      }
    }
  }
};

Level::Level(const char* filename) {
  load(filename);
  createPerimeter(*this);
  createCheckpoints(*this);
}

void Level::load(const char* filename) {
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

bool Level::isWall(int x, int y) {
  return map[y][x] == 0;
}

void Level::draw() {
  int rows = map.size();
  int cols = map[0].size();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int wx = x * tile_size;
      int wy = y * tile_size;
      if (map[y][x] == 1) {
        DrawRectangle(wx, wy, tile_size, tile_size, WHITE);
      } else if (map[y][x] == 2) {
        DrawRectangle(wx, wy, tile_size, tile_size, LIGHTGRAY);
      }
    }
  }
}
