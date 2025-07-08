#include <cfloat>
#include <cstdio>
#define RAYGUI_IMPLEMENTATION

#include <raylib.h>

#include <string>
#include <unordered_map>

#include "conf.h"
#include "level.h"
#include "player.h"
#include "raygui.h"

using conf::win, conf::SIZE, conf::ROWS, conf::COLS;

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
  };

  std::vector<std::vector<char>> map;
  Rectangle start;
  Rectangle finish;
  std::vector<struct Circle> obstacles;
  std::vector<Rectangle> checkpoints;
  std::vector<Coin> coins;
  Shape current_shape = Ball;
  vec2 mouse;

 public:
  LevelBuilder() : map(conf::ROWS, std::vector<char>(conf::COLS, '.')) {}

  void update() {
    float dt = GetFrameTime();
    mouse = GetMousePosition();
    for (auto& obs : obstacles) obs.update(dt);
  }

  vec2 nearest_snap_point(int r, int c) {
    int x = c * SIZE, y = r * SIZE;

    vec2 points[5] = {
      {(float)x, (float)y},                                // top-left
      {(float)(x + SIZE), (float)y},                       // top-right
      {(float)x, (float)(y + SIZE)},                       // btm-left
      {(float)(x + SIZE), (float)(y + SIZE)},              // btm-right
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

  void draw() {
    int HALF_TILE = SIZE / 2;
    for (int y = 0; y < win.y / HALF_TILE; y++) {
      DrawLine(0, y * HALF_TILE, win.x, y * HALF_TILE, Fade(conf::GRID_COLOR, 0.5));
    }
    for (int x = 0; x < win.x / HALF_TILE; x++) {
      DrawLine(x * HALF_TILE, 0, x * HALF_TILE, win.y, Fade(conf::GRID_COLOR, 0.5));
    }

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

    for (const auto& obs : obstacles) obs.draw();

    std::string text = "floor";
    if (current_shape == Ball) text = "circle";
    else if (current_shape == Check) text = "check";

    if (GuiButton({24, 24, 120, 30}, text.c_str())) {
      current_shape = (Shape)((current_shape + 1) % 3);
      return;
    }

    int r = mouse.y / SIZE, c = mouse.x / SIZE;
    vec2 nearest = nearest_snap_point(r, c);

    // follow mouse
    switch (current_shape) {
      case Floor: {
        DrawRectangle(
          c * SIZE,
          r * SIZE,
          SIZE,
          SIZE,
          (r + c) % 2 == 0 ? conf::TILE_COLORS.first : conf::TILE_COLORS.second
        );
        break;
      }
      case Ball: {
        DrawCircleV(nearest, 10, BLUE);
        break;
      };
      case Check: break;
      default: break;  // Unreachable
    }

    switch (current_shape) {
      case Floor: {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) map[r][c] = '#';
      } break;
      case Ball: {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
          vec2 t = to_tiled(nearest);
          printf("(%f, %f)\n", t.x, t.y);
          Bounds bounds = {
            .min = nearest - 10 * SIZE,
            .max = nearest + 10 * SIZE,
          };
          obstacles.push_back(Circle{.pos = nearest, .move = std::make_unique<Linear>(vec2(1,0), 200, bounds)});
          printf("%zu\n", obstacles.size());
        }
      } break;
      case Check: break;
      default: break;  // Unreachable
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      switch (current_shape) {
        case Floor: map[r][c] = '.'; break;
        case Ball: break;
        case Check: break;
        default: break;  // Unreachable
      }
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

  Screen screen = Builder;
  LevelManager level_manager;
  Level* level = nullptr;

  AssetManager asset_manager;
  LevelBuilder builder;

 public:
  Game() : level_manager(2) {
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
