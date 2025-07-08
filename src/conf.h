#pragma once

#include "vec2.h"

namespace conf {

const vec2 win = {1280, 720};
const int SIZE = 40;
const int COLS = win.x / SIZE;
const int ROWS = win.y / SIZE;
const Color BG_COLOR = GetColor(0x67a0bfff);
const Color GRID_COLOR = GetColor(0xbcc2beff);
const std::pair<Color, Color> TILE_COLORS = {GetColor(0xe3e3e3ff), GetColor(0xc7c7c7ff)};
const Color CHECKPOINT_COLOR = GetColor(0x91eda9ff);

};  // namespace conf
