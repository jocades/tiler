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
