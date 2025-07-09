#include "level.h"

#include <fstream>

#include "conf.h"
#include "serde.h"

using conf::SIZE, conf::TILE_COLORS, conf::CHECKPOINT_COLOR;

Linear::Linear(vec2 dir, float speed, Bounds bounds) : dir(dir), speed(speed), bounds(bounds) {}

void Linear::update(float dt, vec2& pos) {
  pos += dir * speed * dt;
  if (dir.x != 0 && (pos.x <= bounds.min.x || pos.x >= bounds.max.x)) dir.x *= -1;
  if (dir.y != 0 && (pos.y <= bounds.min.y || pos.y >= bounds.max.y)) dir.y *= -1;
}

void Circle::update(float dt) {
  move->update(dt, pos);
}

void Circle::draw() const {
  DrawCircleV(pos, radius, BLUE);
  DrawCircleLinesV(pos, radius, BLACK);
}

Coin::Coin(vec2 pos) : pos(pos) {}

void Coin::draw() const {
  DrawCircleV(pos, radius, YELLOW);
  DrawCircleLinesV(pos, radius, BLACK);
}

vec2 tiled(vec2 v) {
  return v * conf::SIZE;
}

Rectangle tiled(Rectangle r) {
  r.x *= SIZE;
  r.y *= SIZE;
  r.width *= SIZE;
  r.height *= SIZE;
  return r;
}

Level::Level(int id) : map(conf::ROWS, std::vector<char>(conf::COLS, 0)) {
  std::filesystem::path path = "levels";
  path.append(std::to_string(id));

  std::ifstream f(path / "data.json");
  if (f.is_open()) {
    json j = json::parse(f);
    start = tiled(j["start"].template get<Rectangle>());
    finish = tiled(j["finish"].template get<Rectangle>());
    obstacles = j["balls"].template get<std::vector<Circle>>();
    if (j.contains("coins")) {
      for (auto& c : j["coins"]) coins.emplace_back(tiled(c.template get<vec2>()));
    }
    std::cout << coins.size() << '\n';
    f.close();
  }

  std::ifstream file(path / "map.txt");
  if (file.is_open()) {
    std::string line;
    for (size_t y = 0; std::getline(file, line); y++) {
      for (size_t x = 0; x < line.size(); x++) {
        map[y][x] = line[x];
      }
    }
    file.close();
  }
}

char Level::get(int row, int col) const {
  if (row < 0 || row >= conf::ROWS || col < 0 || col >= conf::COLS) return '.';
  return map[row][col];
}

void Level::set_player(vec2& pos, vec2 size) {
  Rectangle check = current_checkpoint == -1 ? start : checkpoints[current_checkpoint];
  pos.x = check.x + check.width / 2 - size.x / 2;
  pos.y = check.y + check.height / 2 - size.y / 2;
}

void Level::update(float dt) {
  for (auto& obs : obstacles) obs.update(dt);
}

void Level::draw() const {
  for (size_t y = 0; y < map.size(); y++) {
    for (size_t x = 0; x < map[0].size(); x++) {
      if (map[y][x] == '#') {
        DrawRectangle(
          x * SIZE,
          y * SIZE,
          SIZE,
          SIZE,
          (x + y) % 2 == 0 ? TILE_COLORS.first : TILE_COLORS.second
        );
      }
    }
  }

  DrawRectangleRec(start, CHECKPOINT_COLOR);
  DrawRectangleRec(finish, CHECKPOINT_COLOR);

  for (const auto& obs : obstacles) obs.draw();
  for (const auto& coin : coins) coin.draw();
}

LevelManager::LevelManager(int level_count) {
  for (int id = 1; id <= level_count; id++) {
    levels.emplace_back(id);
  }
}

Level* LevelManager::get(size_t level_num) {
  if (level_num > levels.size()) return nullptr;
  index = level_num - 1;
  return current();
}

Level* LevelManager::current() {
  return &levels[index];
}

Level* LevelManager::next() {
  index++;
  if (index < levels.size()) return current();
  return nullptr;
}

// static void createPerimeter(Level& level) {
//   int rows = level.map.size();
//   int cols = level.map[0].size();
//
//   level.perimeter.clear();
//
//   for (int y = 0; y < rows; y++) {
//     for (int x = 0; x < cols; x++) {
//       if (level.map[y][x] == 0) continue;
//
//       float wx = x * tile_size;
//       float wy = y * tile_size;
//
//       bool is_edge = (x == 0 || x == cols - 1 || y == 0 || y == rows - 1);
//       bool adj = level.isWall(x - 1, y) || level.isWall(x + 1, y) || level.isWall(x, y - 1) ||
//                  level.isWall(x, y + 1);
//
//       if (is_edge || adj) {
//         if (y == 0 || level.map[y - 1][x] == 0) {
//           level.perimeter.push_back({{wx, wy}, {wx + tile_size, wy}});
//         }
//         if (y == rows - 1 || level.map[y + 1][x] == 0) {
//           level.perimeter.push_back({{wx, wy + tile_size}, {wx + tile_size, wy + tile_size}});
//         }
//         if (x == 0 || level.map[y][x - 1] == 0) {
//           level.perimeter.push_back({{wx, wy}, {wx, wy + tile_size}});
//         }
//         if (x == cols - 1 || level.map[y][x + 1] == 0) {
//           level.perimeter.push_back({{wx + tile_size, wy}, {wx + tile_size, wy + tile_size}});
//         }
//       }
//     }
//   }
// }
