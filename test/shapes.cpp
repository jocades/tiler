#include <raylib.h>

#include <cmath>
#include <cstdio>

#define RAYGUI_IMPLEMENTATION

#include "../src/conf.h"
#include "../src/vec2.h"
#include "raygui.h"

using conf::win;

int main() {
  InitWindow(win.x, win.y, "Shapes");

  vec2 center = {win.x / 2, win.y / 2};
  float angle = PI;
  float offset = 20;

  float speed = 1;

  std::vector<vec2> group;

  for (int i = 1; i < 3; i++) {
    group.push_back({center.x + (i * offset) * cos(angle), center.y + (i * offset) * sin(angle)});
  }

  int active = 0;

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    angle += speed * dt;
    for (size_t i = 0; i < group.size(); i++) {
      vec2& v = group[i];
      v.x = center.x + ((i + 1) * offset) * cos(angle);
      v.y = center.y + ((i + 1) * offset) * sin(angle);
    }

    BeginDrawing();

    if (GuiDropdownBox({24, 24, 120, 30}, "Drop", &active, true)) {
      printf("clicked!\n");
    }

    // if (GuiButton({24, 24, 120, 30}, "Click me!")) {
    //   std::printf("clicked!\n");
    // };

    ClearBackground(GetColor(0x282828ff));
    DrawCircleLinesV(center, offset, WHITE);
    DrawCircleV(center, 5, RED);

    for (const auto& v : group) {
      DrawCircleV(v, 10, BLUE);
    }

    EndDrawing();
  }

  CloseWindow();
}
