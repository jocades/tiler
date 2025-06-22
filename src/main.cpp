#include <cstdio>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "json.hpp"
#include "raylib.h"
#include "vec2.h"

using json = nlohmann::json;

const vec2 win = {1280, 720};
const int SIZE = 40;
const int COLS = win.x / SIZE;
const int ROWS = win.y / SIZE;

void to_json(json& j, const vec2& v) {
  j = json::array({v.x, v.y});
}

void from_json(const json& j, vec2& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
}

void from_json(const json& j, Rectangle& r) {
  j.at(0).get_to(r.x);
  j.at(1).get_to(r.y);
  j.at(2).get_to(r.width);
  j.at(3).get_to(r.height);
}

vec2 tiled(vec2 v) {
  return v * SIZE;
}

Rectangle tiled(Rectangle r) {
  r.x *= SIZE;
  r.y *= SIZE;
  r.width *= SIZE;
  r.height *= SIZE;
  return r;
}

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

void from_json(const json& j, Bounds& b) {
  j.at("min").get_to(b.min);
  j.at("max").get_to(b.max);
}

class Linear : public Move {
 public:
  vec2 dir;
  float speed;
  Bounds bounds;

  Linear(vec2 dir, float speed, Bounds bounds) : dir(dir), speed(speed), bounds(bounds) {}

  void update(float dt, vec2& pos) override {
    pos += dir * speed * dt;
    if (dir.x != 0 && (pos.x <= bounds.min.x || pos.x >= bounds.max.x)) dir.x *= -1;
    if (dir.y != 0 && (pos.y <= bounds.min.y || pos.y >= bounds.max.y)) dir.y *= -1;
  }
};

struct Circle {
  vec2 pos;
  float radius = 10;
  std::unique_ptr<Move> move;

  // Circle(vec2 pos, std::unique_ptr<Move> move) : pos(pos), move(std::move(move)) {}

  void update(float dt) {
    move->update(dt, pos);
  }

  void draw() const {
    DrawCircleV(pos, radius, BLUE);
    DrawCircleLinesV(pos, radius, BLACK);
  }
};

void from_json(const json& j, Circle& c) {
  j.at("pos").get_to(c.pos);
  c.pos *= SIZE;

  if (j["move"]["kind"] == "linear") {
    Bounds bounds = j["move"]["bounds"].template get<Bounds>();
    bounds.min = tiled(bounds.min);
    bounds.max = tiled(bounds.max);
    c.move = std::make_unique<Linear>(
      j["move"]["dir"].template get<vec2>().norm(),
      j["move"]["speed"].template get<float>(),
      bounds
    );
  }
}

struct Coin {
  vec2 pos;
  float radius = 7.5;

  void draw() const {
    DrawCircleV(pos, radius, YELLOW);
    DrawCircleLinesV(pos, radius, BLACK);
  }
};

class Level {
 public:
  std::vector<std::vector<int>> map;
  Rectangle start;
  Rectangle finish;
  std::vector<Circle> obstacles;
  std::vector<Rectangle> checkpoints;
  std::vector<Coin> coins;
  int current_checkpoint = -1;

  const std::pair<Color, Color> TILE_COLORS = {GetColor(0xe3e3e3ff), GetColor(0xc7c7c7ff)};
  const Color CHECKPOINT_COLOR = GetColor(0x91eda9ff);

  Level(int id) : map(ROWS, std::vector<int>(COLS, 0)) {
    std::filesystem::path path = "levels";
    path.append(std::to_string(id));

    std::ifstream f(path / "data.json");
    if (f.is_open()) {
      json j = json::parse(f);
      start = tiled(j["start"].template get<Rectangle>());
      finish = tiled(j["finish"].template get<Rectangle>());
      obstacles = j["balls"].template get<std::vector<Circle>>();
      f.close();
    }

    std::ifstream file(path / "map.txt");
    if (file.is_open()) {
      std::string line;
      for (size_t y = 0; std::getline(file, line); y++) {
        for (size_t x = 0; x < line.size(); x++) {
          map[y][x] = line[x] - '0';
        }
      }
      file.close();
    }
  }

  int get(int row, int col) const {
    return map[row][col];
  }

  void set_player(vec2& pos, vec2 size) {
    Rectangle check = current_checkpoint == -1 ? start : checkpoints[current_checkpoint];
    pos.x = check.x + check.width / 2 - size.x / 2;
    pos.y = check.y + check.height / 2 - size.y / 2;
  }

  void update(float dt) {
    for (auto& obs : obstacles) obs.update(dt);
  }

  void draw() const {
    for (size_t y = 0; y < map.size(); y++) {
      for (size_t x = 0; x < map[0].size(); x++) {
        if (map[y][x] == 1) {
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
};

bool sweep_aabb(vec2& pos, vec2 size, vec2 delta, Level* level) {
  if (delta.x != 0) {
    float nx = pos.x + delta.x;
    float x_edge = delta.x > 0 ? nx + size.x - 1 : nx;
    int c = x_edge / SIZE;

    int row_start = pos.y / SIZE;
    int row_end = (pos.y + size.y - 1) / SIZE;

    for (int r = row_start; r <= row_end; r++) {
      if (level->get(r, c) == 0) {
        // Snap to edge of tile
        if (delta.x > 0) pos.x = c * SIZE - size.x;
        else pos.x = (c + 1) * SIZE;
        delta.x = 0;
        break;
      }
    }

    pos.x += delta.x;
  }

  if (delta.y != 0) {
    float ny = pos.y + delta.y;
    float y_edge = delta.y > 0 ? ny + size.y - 1 : ny;
    int r = y_edge / SIZE;

    int col_start = pos.x / SIZE;
    int col_end = (pos.x + size.x - 1) / SIZE;

    for (int c = col_start; c <= col_end; c++) {
      if (level->get(r, c) == 0) {
        if (delta.y > 0) pos.y = r * SIZE - size.y;
        else pos.y = (r + 1) * SIZE;
        delta.y = 0;
        break;
      }
    }

    pos.y += delta.y;
  }

  return true;
}

class FadeAnimation {
 public:
  float frame_delay;
  int index = 0;
  float timer = 0;
  bool done = false;
  std::vector<float> frames;

  FadeAnimation(float duration, float steps) : frame_delay(duration / steps) {
    for (int i = steps; i >= 0; i--) {
      frames.push_back(i / (steps - 1));
    }
  }

  void reset() {
    index = 0;
    timer = 0;
    done = false;
  }

  float current_frame() const {
    return frames[index];
  }

  void update(float dt) {
    if (done) return;
    timer += dt;
    if (timer >= frame_delay) {
      timer = 0;
      index++;
      if (index >= (int)frames.size()) {
        index = frames.size() - 1;
        done = true;
      }
    }
  }
};

class Player {
 public:
  vec2 pos{win.x / 2, win.y / 2};
  vec2 dir;
  vec2 size = {25, 25};
  float speed = 200;
  float dead = false;
  FadeAnimation fade{1, 21};

  Player() = default;

  Rectangle rect() const {
    return {pos.x, pos.y, size.x, size.y};
  }

  void input() {
    dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
    dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
    dir = dir.norm();
  }

  void move(float dt, Level* level) {
    vec2 delta = dir * speed * dt;
    sweep_aabb(pos, size, delta, level);
  }

  void update(float dt, Level* level) {
    if (dead) {
      fade.update(dt);
      if (fade.done) {
        dead = false;
        level->set_player(pos, size);
      }
      return;
    }

    input();
    move(dt, level);
  }

  void draw() const {
    float alpha = dead ? fade.current_frame() : 1.0f;
    DrawRectangleV(pos, size, Fade(ORANGE, alpha));
  }
};

class LevelManager {
 public:
  std::vector<Level> levels;
  int index = 0;

  LevelManager() {
    for (int id = 1; id < 3; id++) {
      levels.emplace_back(id);
    }
  }

  Level* current() {
    return &levels[index];
  }

  Level* next() {
    index++;
    if (index < (int)levels.size()) return current();
    return nullptr;
  }
};

class AssetManager {
 public:
  Music music;
  std::unordered_map<std::string, Sound> sounds;

  void load() {
    music = LoadMusicStream("assets/music/theme.mp3");
    sounds["hit"] = LoadSound("assets/sounds/hit.mp3");
    sounds["collect"] = LoadSound("assets/sounds/collect.mp3");
  }
};

class Game {
 private:
  enum Screen {
    Start,
    Play,
    Done,
  };

  const Color BG_COLOR = GetColor(0x67a0bfff);
  const Color GRID_COLOR = GetColor(0xbcc2beff);

  Player player;
  int deaths = 0;

  Screen screen = Screen::Start;
  LevelManager level_manager;
  Level* level = nullptr;

  AssetManager asset_manager;

 public:
  Game() {
    InitWindow(win.x, win.y, "The Impossible Game");
    InitAudioDevice();
    asset_manager.load();
    PlayMusicStream(asset_manager.music);
  }

  void run() {
    while (!WindowShouldClose()) {
      update();
      draw();
    }
  }

  void update_start() {
    if (IsKeyPressed(KEY_ENTER)) {
      level = level_manager.current();
      level->set_player(player.pos, player.size);
      screen = Play;
    }
  }

  void update_play() {
    float dt = GetFrameTime();

    level->update(dt);
    player.update(dt, level);

    if (player.dead) return;

    Rectangle rect = player.rect();
    for (auto& obs : level->obstacles) {
      if (CheckCollisionCircleRec(obs.pos, obs.radius, rect)) {
        PlaySound(asset_manager.sounds["hit"]);
        deaths += 1;
        player.dead = true;
        player.fade.reset();
        return;
      }

      for (int i = 0; i < (int)level->checkpoints.size(); i++) {
        if (CheckCollisionRecs(rect, level->checkpoints[i])) {
          level->current_checkpoint = i;
        }
      }

      if (level->coins.size() == 0 && CheckCollisionRecs(rect, level->finish)) {
        screen = Done;
      }
    }
  }

  void update_done() {
    if (IsKeyPressed(KEY_ENTER)) {
      if (!(level = level_manager.next())) return;
      level->set_player(player.pos, player.size);
      screen = Play;
    }
  }

  void update() {
    UpdateMusicStream(asset_manager.music);
    switch (screen) {
      case Start: update_start(); break;
      case Play: update_play(); break;
      case Done: update_done(); break;
    }
  }

  void draw_grid(float spacing) {
    for (int x = 0; x < COLS; x++) {
      DrawLine(x * spacing, 0, x * spacing, win.y, GRID_COLOR);
    }
    for (int y = 0; y < ROWS; y++) {
      DrawLine(0, y * spacing, win.x, y * spacing, GRID_COLOR);
    }
  }

  void draw_start() {
    const char* text = "START SCREEN";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(text, win.x / 2 - text_width / 2, win.y / 2 - font_size / 2, font_size, LIGHTGRAY);
  }

  void draw_header() {
    DrawRectangle(0, 0, win.x, SIZE, BLACK);
    int font_size = 20;
    std::string text = "LEVEL: " + std::to_string(1);
    DrawText(
      text.c_str(),
      win.x / 2 - (float)MeasureText(text.c_str(), font_size) / 2,
      SIZE / 2,
      font_size,
      RAYWHITE
    );
    text = "DEATHS: " + std::to_string(deaths);
    DrawText(
      text.c_str(),
      win.x - MeasureText(text.c_str(), font_size) - SIZE,
      SIZE / 2 - font_size / 2,
      font_size,
      RAYWHITE
    );
  }

  void draw_play() {
    draw_grid(SIZE);
    level->draw();
    player.draw();
    draw_header();
  }

  void draw_done() {
    const char* text = "LEVEL COMPLETE";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(text, win.x / 2 - text_width / 2, win.y / 2 - font_size / 2, font_size, LIGHTGRAY);
  }

  void draw() {
    BeginDrawing();
    ClearBackground(BG_COLOR);
    draw_grid(SIZE);
    switch (screen) {
      case Start: draw_start(); break;
      case Play: draw_play(); break;
      case Done: draw_done(); break;
    }
    DrawFPS(0, 0);
    EndDrawing();
  }

  ~Game() {
    CloseAudioDevice();
    CloseWindow();
  }
};

int main() {
  Game game;
  game.run();
}

// struct Game {
//   enum Screen {
//     Start,
//     Gameplay,
//     Complete,
//     GameOver,
//   };
//
//   Level level;
//   Player player = {.size = {25, 25}, .speed = 200};
//   Screen current_screen = Start;
//
//   Game() {
//     SetTraceLogLevel(LOG_TRACE);
//     TraceLog(LOG_TRACE, "cols=%d rows=%d", cols, rows);
//     InitWindow(screen.x, screen.y, "Tiler");
//     SetTargetFPS(144);
//   }
//
//   void updateStartScreen() {
//     level.setPlayer(player);
//
//     if (IsKeyPressed(KEY_ENTER)) {
//       current_screen = Gameplay;
//     }
//   }
//
//   void updateGameplayScreen() {
//     float dt = GetFrameTime();
//
//     player.update(dt);
//     auto rect = player.rect();
//
//     // Vector2 sides[4] = {
//     //   {rect.x + rect.width / 2, rect.y},                // top
//     //   {rect.x + rect.width, rect.y + rect.height / 2},  // right
//     //   {rect.x + rect.width / 2, rect.y + rect.height},  // bottom
//     //   {rect.x, rect.y + rect.height / 2},               // left
//     // };
//     //
//     // for (size_t i = 0; i < level.perimeter.size() - 1; i++) {
//     //   for (const auto& corner : sides) {
//     //     if (CheckCollisionPointLine(corner, level.perimeter[i], level.perimeter[i + 1], 2)) {
//     //       if (player.dir.x == 1) {
//     //         player.pos.x = level.perimeter[i].x - player.size.x;
//     //       }
//     //       if (player.dir.x == -1) {
//     //         player.pos.x = level.perimeter[i].x;
//     //       }
//     //       if (player.dir.y == 1) {
//     //         player.pos.y = level.perimeter[i].y - player.size.y;
//     //       }
//     //       if (player.dir.y == -1) {
//     //         player.pos.y = level.perimeter[i].y;
//     //       }
//     //     }
//     //   }
//     // }
//
//     if (player.pos.x <= level.bounds.x) {
//       player.pos.x = level.bounds.x;
//     }
//     if (player.pos.x >= level.bounds.x + level.bounds.width - player.size.x) {
//       player.pos.x = level.bounds.x + level.bounds.width - player.size.x;
//     }
//     if (player.pos.y <= level.bounds.y) {
//       player.pos.y = level.bounds.y;
//     }
//     if (player.pos.y >= level.bounds.y + level.bounds.height - player.size.y) {
//       player.pos.y = level.bounds.y + level.bounds.height - player.size.y;
//     }
//
//     for (auto& obs : level.obstacles) {
//       if (CheckCollisionCircleRec(obs.pos, obs.radius, rect)) {
//         current_screen = GameOver;
//         return;
//       }
//
//       if (obs.pos.y <= level.bounds.y + half_tile ||
//           obs.pos.y >= level.bounds.y + level.bounds.height - half_tile) {
//         obs.dir.y = -obs.dir.y;
//       }
//
//       obs.pos += obs.dir * obs.speed * dt;
//     }
//
//     for (size_t i = 0; i < level.checkpoints.size(); i++) {
//       if (CheckCollisionRecs(rect, level.checkpoints[i].rect())) {
//         level.current_checkpoint = i;
//         break;
//       }
//     }
//
//     std::erase_if(level.coins, [&rect](const Circle& coin) {
//       return CheckCollisionCircleRec(coin.pos, coin.radius, rect);
//     });
//
//     if (level.coins.empty() && CheckCollisionRecs(rect, level.finish.rect())) {
//       current_screen = Complete;
//     }
//   }
//
//   void updateCompleteScreen() {
//     if (IsKeyPressed(KEY_ENTER)) {
//       level.current_checkpoint = -1;
//       level.reset(player);
//       current_screen = Gameplay;
//     }
//   }
//
//   void updateGameOverScreen() {
//     if (IsKeyPressed(KEY_ENTER)) {
//       level.reset(player);
//       current_screen = Gameplay;
//     }
//   }
//
//   void update() {
//     switch (current_screen) {
//       case Start: updateStartScreen(); break;
//       case Gameplay: updateGameplayScreen(); break;
//       case Complete: updateCompleteScreen(); break;
//       case GameOver: updateGameOverScreen(); break;
//     }
//   }
//
//   void drawStartScreen() {
//     const char* text = "START SCREEN";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void drawGameplayScreen() {
//     DrawRectangleRec(level.start.rect(), LIGHTGRAY);
//     DrawRectangleRec(level.finish.rect(), LIGHTGRAY);
//
//     for (auto& checkpoint : level.checkpoints) {
//       DrawRectangleRec(checkpoint.rect(), CHECKPOINT);
//     }
//
//     player.draw();
//
//     for (const auto& obs : level.obstacles) {
//       DrawCircleV(obs.pos, obs.radius, BLUE);
//     }
//
//     for (const auto& coin : level.coins) {
//       DrawCircleV(coin.pos, coin.radius, YELLOW);
//       DrawCircleLinesV(coin.pos, coin.radius, BLACK);
//     }
//
//     for (size_t i = 0; i < level.perimeter.size() - 1; i++) {
//       DrawLineEx(level.perimeter[i], level.perimeter[i + 1], 4, BLACK);
//     }
//   }
//
//   void drawCompleteScreen() {
//     const char* text = "LEVEL COMPLETE";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void drawGameOverScreen() {
//     const char* text = "GAME OVER";
//     float font_size = 40;
//     float text_width = MeasureText(text, font_size);
//     DrawText(
//       text,
//       screen.x / 2 - text_width / 2,
//       screen.y / 2 - font_size / 2,
//       font_size,
//       LIGHTGRAY
//     );
//   }
//
//   void draw() {
//     BeginDrawing();
//     ClearBackground(RAYWHITE);
//     drawGrid();
//     switch (current_screen) {
//       case Start: drawStartScreen(); break;
//       case Gameplay: drawGameplayScreen(); break;
//       case Complete: drawCompleteScreen(); break;
//       case GameOver: drawGameOverScreen(); break;
//     }
//     DrawFPS(0, 0);
//     EndDrawing();
//   }
//
//   void run() {
//     while (!WindowShouldClose()) {
//       update();
//       draw();
//     }
//   }
//
//   ~Game() {
//     CloseWindow();
//   }
// };
