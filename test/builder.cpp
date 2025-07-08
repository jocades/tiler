#include <raylib.h>

#include <algorithm>
#include <fstream>
#include <iostream>

const Color CHECKPOINT = GetColor(0x91eda9ff);

Vector2 screen{1280, 720};
const float tile_size = 40.0f;
const int cols = screen.x / tile_size;
const int rows = screen.y / tile_size;

enum Kind {
  Floor = 1,
  Checkpoint
};
Kind current_kind = Floor;

std::vector<std::vector<int>> map(rows, std::vector<int>(cols, 0));

bool inbounds(int x, int y) {
  return 0 <= x && x < cols && 0 <= y && y < rows;
}

bool isWall(int x, int y) {
  if (!inbounds(x, y)) return true;
  return map[y][x] == 0;
}

bool isCheckpoint(int x, int y) {
  return map[y][x] == 2;
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

void drawMap() {
  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      int wx = x * tile_size;
      int wy = y * tile_size;
      if (map[y][x] == Kind::Floor) {
        DrawRectangle(wx, wy, tile_size, tile_size, (y + x) % 2 == 0 ? DARKGRAY : GRAY);
      } else if (map[y][x] == Kind::Checkpoint) {
        DrawRectangle(wx, wy, tile_size, tile_size, CHECKPOINT);
      }
    }
  }
}

void drawGrid(float sub = false) {
  for (int x = 0; x < cols; x++) {
    DrawLine(x * tile_size, 0, x * tile_size, screen.y, DARKGRAY);
  }
  for (int y = 0; y < rows; y++) {
    DrawLine(0, y * tile_size, screen.x, y * tile_size, DARKGRAY);
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

std::vector<std::pair<Vector2, Vector2>> perimeter;

void calculatePerimeter() {
  perimeter.clear();

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (map[y][x] == 0) continue;

      float wx = x * tile_size;
      float wy = y * tile_size;

      bool is_edge = (x == 0 || x == cols - 1 || y == 0 || y == rows - 1);  // Edge of the map
      bool adj = isWall(x - 1, y) || isWall(x + 1, y) || isWall(x, y - 1) || isWall(x, y + 1);

      if (is_edge || adj) {
        if (y == 0 || map[y - 1][x] == 0) {
          perimeter.push_back({{wx, wy}, {wx + tile_size, wy}});
        }
        if (y == rows - 1 || map[y + 1][x] == 0) {
          perimeter.push_back({{wx, wy + tile_size}, {wx + tile_size, wy + tile_size}});
        }
        if (x == 0 || map[y][x - 1] == 0) {
          perimeter.push_back({{wx, wy}, {wx, wy + tile_size}});
        }
        if (x == cols - 1 || map[y][x + 1] == 0) {
          perimeter.push_back({{wx + tile_size, wy}, {wx + tile_size, wy + tile_size}});
        }
      }
    }
  }
}

std::pair<int, int> directions[4] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};

std::vector<Rectangle> checkpoints;
std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
int current_checkpoint = -1;

void calculateCheckpoints() {
  checkpoints.clear();
  std::fill(visited.begin(), visited.end(), std::vector<bool>(cols, false));

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols; x++) {
      if (map[y][x] == Kind::Checkpoint && !visited[y][x]) {
        int start_x = x, start_y = y;
        int end_x = x, end_y = y;

        while (end_x + 1 < cols && map[y][end_x + 1] == Kind::Checkpoint) {
          end_x++;
        }

        bool valid = true;
        while (valid && end_y + 1 < rows) {
          for (int i = start_x; i <= end_x; i++) {
            if (map[end_y + 1][i] != Kind::Checkpoint) {
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

        checkpoints.push_back({
          start_x * tile_size,
          start_y * tile_size,
          (end_x - start_x + 1) * tile_size,
          (end_y - start_y + 1) * tile_size,
        });
      }
    }
  }
}

void resetMap() {
  for (auto& row : map) {
    std::fill(row.begin(), row.end(), 0);
  }
  calculatePerimeter();
  calculateCheckpoints();
  current_checkpoint = -1;
}

void drawPerimeter() {
  for (const auto& edge : perimeter) {
    DrawLineV(edge.first, edge.second, BLACK);
  }
}

struct Coord {
  int x;
  int y;
};

struct Player {
  Vector2 pos = {0, 0};
  Vector2 dir = {0, 0};
  float size = 25.0f;
  float speed = 200;

  Rectangle rect() const {
    return {
      .x = pos.x,
      .y = pos.y,
      .width = size,
      .height = size,
    };
  }

  Coord coord() {
    return {
      int(pos.x / tile_size),
      int(pos.y / tile_size),
    };
  }

  void update(float dt) {
    dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
    normalize(dir);

    Coord c = coord();

    pos += dir * speed * dt;

    int nx = (pos.x + (dir.x > 0 ? size : 0)) / tile_size;
    if (isWall(nx, c.y)) {
      if (dir.x > 0) pos.x = nx * tile_size - size;
      if (dir.x < 0) pos.x = (nx + 1) * tile_size;
    };

    int ny = (pos.y + (dir.y > 0 ? size : 0)) / tile_size;
    if (isWall(c.x, ny)) {
      if (dir.y > 0) pos.y = ny * tile_size - size;
      if (dir.y < 0) pos.y = (ny + 1) * tile_size;
    }
  }
};

std::vector<Vector2> obstacles;
bool adding_obstacles = false;

int main() {
  InitWindow(screen.x, screen.y, "Maps");

  bool play = false;
  Player player{};

  printf("cols=%d rows=%d\n", cols, rows);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    if (play) {
      player.update(dt);
    }

    for (int i = 0; i < int(checkpoints.size()); i++) {
      Rectangle checkpoint = checkpoints[i];
      if (CheckCollisionRecs(player.rect(), checkpoint)) {
        if (i != current_checkpoint) {
          printf("Checkpoint %d\n", i);
        }
        current_checkpoint = i;
        break;
      }
    }

    Vector2 mouse_pos = GetMousePosition();
    int x = mouse_pos.x / tile_size;
    int y = mouse_pos.y / tile_size;

    if (inbounds(x, y)) {
      if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (!adding_obstacles) {
          if (map[y][x] != current_kind) {
            map[y][x] = current_kind;
            calculatePerimeter();
            calculateCheckpoints();
            printf("checkpoints = %zu\n", checkpoints.size());
          }
        } else {
          obstacles.push_back({x * tile_size + tile_size / 2, y * tile_size + tile_size / 2});
        }
      }
      if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (map[y][x] != 0) {
          map[y][x] = 0;
          calculatePerimeter();
          calculateCheckpoints();
        }
      }
    }

    if (IsKeyPressed(KEY_S)) saveMap("map.txt");
    if (IsKeyPressed(KEY_L)) loadMap("map.txt");

    if (IsKeyPressed(KEY_F)) current_kind = Kind::Floor;
    if (IsKeyPressed(KEY_C)) current_kind = Kind::Checkpoint;
    if (IsKeyPressed(KEY_O)) adding_obstacles = !adding_obstacles;

    if (IsKeyPressed(KEY_R)) resetMap();
    if (IsKeyPressed(KEY_P)) {
      player.pos = mouse_pos;
      play = !play;
    };

    BeginDrawing();
    ClearBackground(RAYWHITE);

    drawGrid(true);
    drawMap();
    drawPerimeter();

    if (play) {
      DrawRectangleV(player.pos, {player.size, player.size}, ORANGE);
    }

    for (const auto& obs : obstacles) {
      DrawCircleV(obs, 12.5, BLUE);
    }

    DrawFPS(0, 0);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
