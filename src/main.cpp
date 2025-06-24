#include <raylib.h>

#include <string>
#include <unordered_map>

#include "conf.h"
#include "level.h"
#include "player.h"

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
