#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include "../src/conf.h"
#include "../src/vec2.h"
#include "raygui.h"

using conf::win;

int main() {
  InitWindow(win.x, win.y, "Shapes");

  float value = 0.5;
  float timer = 0;
  int spinner = 5;

  char* text;
  bool a = false;
  bool b = true;
  bool c = false;
  int current = 0;

  while (!WindowShouldClose()) {
    timer += GetFrameTime();
    BeginDrawing();

    ClearBackground(RAYWHITE);
    int v = GuiSliderBar({25, 25, 100, 50}, "L", "R", &value, 0, 1);

    GuiLabel({500, 100, 100, 50}, "MODE");

    if (GuiToggle({500, 130, 100, 30}, "CELL", &a)) printf("a\n");
    GuiToggle({500, 162, 100, 30}, "BALL", &b);

    // if (b) {
    //   a = false;
    // }
    // if (a) {
    //   b = false;
    // } else {
    //   b = true;
    // }

    GuiButton({500, 200, 100, 30}, "BTN");

    GuiLabel({200, 150, 100, 25}, "Bounds");
    GuiSpinner({200, 200, 100, 25}, "X", &spinner, 0, 10, false);
    GuiSpinner({200, 250, 100, 25}, "y", &spinner, 0, 10, false);

    if (timer >= 2.0f) {
      printf("%f %d\n", value, spinner);
      timer = 0;
    }
    EndDrawing();
  }

  CloseWindow();
}
