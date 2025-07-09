#include <cctype>
#include <cfloat>
#include <cstdio>
#include <fstream>
#include <ostream>
#define RAYGUI_IMPLEMENTATION

#include <raylib.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "conf.h"
#include "json.h"
#include "level.h"
#include "player.h"
#include "raygui.h"
#include "serde.h"

using conf::win, conf::SIZE, conf::ROWS, conf::COLS, nlohmann::json;

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

class LevelBuilder {
 private:
  enum Shape {
    Floor,
    Ball,
    Check,
    Start,
    Finish,
    Coin_
  };

  std::vector<std::vector<char>> map;
  Rectangle start;
  Rectangle finish;
  std::vector<Circle> obstacles;
  std::vector<Circle> live_obstacles;
  std::vector<Rectangle> checkpoints;
  std::vector<Coin> coins;
  int current_shape = Floor;
  vec2 mouse;
  Player* player;
  int current_checkpoint = -1;
  bool is_playing = false;
  int active = 0;
  int minx{}, miny{}, maxx{}, maxy{};
  float timer;

  void save() {
    std::ofstream f("test.txt");
    if (!f.is_open()) return;
    for (auto& row : map) {
      std::string line(row.data(), row.size());
      line.push_back('\n');
      f << line;
    }

    std::ofstream d("data.json");
    if (!d.is_open()) return;
    json j = json::object();
    j["start"] = {start.x / SIZE, start.y / SIZE, start.width / SIZE, start.height / SIZE};
    j["finish"] = {finish.x / SIZE, finish.y / SIZE, finish.width / SIZE, finish.height / SIZE};
    for (auto& obs : obstacles) j["balls"].push_back(obs);
    for (auto& coin : coins) j["coins"].push_back(coin.pos);

    d << j.dump(2);
  }

  vec2 nearest_snap_point(int r, int c) {
    int x = c * SIZE, y = r * SIZE;

    vec2 points[] = {
      {(float)x, (float)y},                                // top-left
      {(float)(x + SIZE), (float)y},                       // top-right
      {(float)(x + SIZE * 0.5), float(y)},                 // top-center
      {(float)x, (float)(y + SIZE)},                       // btm-left
      {(float)(x + SIZE), (float)(y + SIZE)},              // btm-right
      {(float)(x + SIZE * 0.5), (float)(y + SIZE)},        // btm-center
      {(float)x, (float)(y + SIZE * 0.5)},                 // left-center
      {(float)(x + SIZE), (float)(y + SIZE * 0.5)},        // right-center
      {(float)(x + SIZE * 0.5), (float)(y + SIZE * 0.5)},  // center
    };

    float min_dist = FLT_MAX;
    vec2 nearest{};

    for (const vec2& point : points) {
      vec2 delta = point - mouse;
      float dist = delta.x * delta.x + delta.y * delta.y;
      if (dist < min_dist) {
        min_dist = dist;
        nearest = point;
      }
    }

    return nearest;
  }

  // Return the grid position allowing for '0.5' decimal points.
  vec2 to_tiled(vec2 pos) {
    int row = pos.y / SIZE, col = pos.x / SIZE;
    float local_x = fmodf(pos.x, SIZE);
    float local_y = fmodf(pos.y, SIZE);
    float tx = (local_x == SIZE * 0.5f) ? col + 0.5f : (pos.x == (col + 1)) * SIZE ? col + 1 : col;
    float ty = (local_y == SIZE * 0.5f) ? row + 0.5f : (pos.y == (row + 1) * SIZE) ? row + 1 : row;
    return {tx, ty};
  }

 public:
  LevelBuilder(Player* player)
      : map(conf::ROWS, std::vector<char>(conf::COLS, '.')), player(player) {}

  void update() {
    float dt = GetFrameTime();
    timer += dt;
    mouse = GetMousePosition();
    if (is_playing) {
      for (auto& obs : live_obstacles) obs.update(dt);
    }

    if (timer >= 1.0f) {
      timer = 0;
      // std::cout << "min: " << bounds.min << " max: " << bounds.max << '\n';
    }
  }

  void draw_grid() {
    int HALF_TILE = SIZE / 2;
    for (int y = 0; y < win.y / HALF_TILE; y++) {
      DrawLine(0, y * HALF_TILE, win.x, y * HALF_TILE, Fade(conf::GRID_COLOR, 0.5));
    }
    for (int x = 0; x < win.x / HALF_TILE; x++) {
      DrawLine(x * HALF_TILE, 0, x * HALF_TILE, win.y, Fade(conf::GRID_COLOR, 0.5));
    }
  }

  void draw_ball_bounds(const Circle& circle) {
    switch (circle.move->kind) {
      case Move::Linear: {
        Linear* linear = (Linear*)circle.move.get();
        const Bounds& bounds = linear->bounds;
        DrawLine(bounds.min.x, bounds.min.y, bounds.max.x, bounds.max.y, RED);
      } break;
    }
  }

  void draw_level() {
    for (size_t y = 0; y < map.size(); y++) {
      for (size_t x = 0; x < map[0].size(); x++) {
        if (map[y][x] == '#') {
          DrawRectangle(
            x * SIZE,
            y * SIZE,
            SIZE,
            SIZE,
            (x + y) % 2 == 0 ? conf::TILE_COLORS.first : conf::TILE_COLORS.second
          );
        }
      }
    }

    DrawRectangleRec(start, conf::CHECKPOINT_COLOR);
    DrawRectangleRec(finish, conf::CHECKPOINT_COLOR);

    for (const auto& obs : is_playing ? live_obstacles : obstacles) {
      obs.draw();
      draw_ball_bounds(obs);
    }

    for (const auto& coin : coins) coin.draw();
  }

  void draw_controls() {
    DrawLine(0, SIZE * 2, win.x - SIZE * 2, SIZE * 2, BLACK);
    DrawLine(win.x - SIZE * 3, SIZE * 2, win.x - SIZE * 3, win.y, BLACK);

    GuiToggleGroup({24, 24, 24, 24}, "F;B;K\nS;F;C", &current_shape);
    switch (current_shape) {
      case Ball: {
        Rectangle top = {24 * 5, 24, 50, 25};
        GuiLabel(top, "MIN");
        top.y += 24;
        top.width = 80;
        GuiSpinner(top, "X", &minx, -50, 50, false);
        top.x += 90;
        GuiSpinner(top, "Y", &miny, -50, 50, false);
        top.y -= 24;
        top.x += 90;
        GuiLabel(top, "MAX");
        top.y += 24;
        GuiSpinner(top, "X", &maxx, -50, 50, false);
        top.x += 90;
        GuiSpinner(top, "Y", &maxy, -50, 50, false);
      } break;
      default: break;
    }

    if (GuiButton({win.x - 24 - 120, 24 + 32, 120, 30}, "Save")) save();
    GuiToggle({win.x - 24 - 120, 24, 120, 30}, "Play", &is_playing);
  }

  void draw() {
    draw_grid();
    draw_level();
    draw_controls();

    if (is_playing) {
      if (obstacles.size() != live_obstacles.size()) {
        live_obstacles = obstacles;
      }
      return;
    }

    // Dont place items if mouse over controls
    if (mouse.x >= win.x - SIZE * 3 || mouse.y <= SIZE * 2) return;

    int r = mouse.y / SIZE, c = mouse.x / SIZE;
    vec2 snap = nearest_snap_point(r, c);

    switch (current_shape) {
      case Floor: {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) map[r][c] = '#';
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) map[r][c] = '.';

        DrawRectangle(
          c * SIZE,
          r * SIZE,
          SIZE,
          SIZE,
          (r + c) % 2 == 0 ? conf::TILE_COLORS.first : conf::TILE_COLORS.second
        );
      } break;

      case Ball: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          Bounds bounds = {
            .min = {snap.x - minx * SIZE, snap.y - miny * SIZE},
            .max = {snap.x + maxx * SIZE, snap.y + maxy * SIZE}
          };
          Circle circle = {
            .pos = snap,
            .move = std::make_shared<Linear>(vec2(1, 0), 200, bounds),
          };
          obstacles.push_back(circle);
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
          std::erase_if(obstacles, [&snap](const Circle& c) { return c.pos == snap; });
        }

        DrawCircleV(snap, 10, BLUE);
        DrawLine(
          snap.x - minx * SIZE,
          snap.y - miny * SIZE,
          snap.x + maxx * SIZE,
          snap.y + maxy * SIZE,
          RED
        );
      } break;

      case Coin_: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          coins.emplace_back(snap);
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
          std::erase_if(coins, [&snap](const Coin& c) { return c.pos == snap; });
        }

        DrawCircleV(snap, 7.5, YELLOW);
      } break;

      case Check: {
        DrawRectangle(c * SIZE, r * SIZE, SIZE, SIZE, conf::CHECKPOINT_COLOR);
      } break;

      case Start: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          start = {(float)c * SIZE, (float)r * SIZE, SIZE * 2, SIZE * 2};
        }
        DrawRectangle(c * SIZE, r * SIZE, SIZE * 2, SIZE * 2, conf::CHECKPOINT_COLOR);
      } break;

      case Finish: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          finish = {(float)c * SIZE, (float)r * SIZE, SIZE * 2, SIZE * 2};
        }
        DrawRectangle(c * SIZE, r * SIZE, SIZE * 2, SIZE * 2, conf::CHECKPOINT_COLOR);
      } break;
    }
  }
};

class Game {
 private:
  enum Screen {
    Start,
    Play,
    Done,
    Builder,
  };

  Player player;
  int deaths = 0;

  Screen screen = Start;
  LevelManager level_manager;
  Level* level = nullptr;

  AssetManager asset_manager;
  LevelBuilder builder;

 public:
  Game() : level_manager(2), builder(&player) {
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
    }

    char key = GetKeyPressed();
    if (isdigit(key)) {
      int level_num = key - '0';
      printf("level = %d\n", level_num);
      level = level_manager.get(level_num);
    }

    if (level != nullptr) {
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

      for (size_t i = 0; i < level->coins.size(); i++) {
        auto& coin = level->coins[i];
        if (CheckCollisionCircleRec(coin.pos, coin.radius, rect)) {
          PlaySound(asset_manager.sounds["collect"]);
          level->coins.erase(level->coins.begin() + i);
        }
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

  void update_builder() {
    builder.update();
  }

  void update() {
    // UpdateMusicStream(asset_manager.music);
    switch (screen) {
      case Start: update_start(); break;
      case Play: update_play(); break;
      case Done: update_done(); break;
      case Builder: update_builder(); break;
    }
  }

  void draw_grid(float spacing) {
    for (int x = 0; x < COLS; x++) {
      DrawLine(x * spacing, 0, x * spacing, win.y, conf::GRID_COLOR);
    }
    for (int y = 0; y < ROWS; y++) {
      DrawLine(0, y * spacing, win.x, y * spacing, conf::GRID_COLOR);
    }
  }

  void draw_start() {
    const char* text = "START SCREEN";
    float font_size = 40;
    float text_width = MeasureText(text, font_size);
    DrawText(text, win.x / 2 - text_width / 2, win.y / 2 - font_size / 2, font_size, LIGHTGRAY);
    if (GuiButton({24, 24, 120, 30}, "Builder")) {
      screen = Builder;
    }
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

  void draw_builder() {
    builder.draw();
  }

  void draw() {
    BeginDrawing();
    ClearBackground(conf::BG_COLOR);
    draw_grid(SIZE);
    switch (screen) {
      case Start: draw_start(); break;
      case Play: draw_play(); break;
      case Done: draw_done(); break;
      case Builder: draw_builder(); break;
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
