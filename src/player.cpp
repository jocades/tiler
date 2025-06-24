#include "player.h"

#include "conf.h"

using conf::SIZE;

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

Rectangle Player::rect() const {
  return {pos.x, pos.y, size.x, size.y};
}

void Player::input() {
  dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
  dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
  dir = dir.norm();
}

void Player::move(float dt, Level* level) {
  vec2 delta = dir * speed * dt;
  sweep_aabb(pos, size, delta, level);
}

void Player::update(float dt, Level* level) {
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

void Player::draw() const {
  float alpha = dead ? fade.current_frame() : 1.0f;
  DrawRectangleV(pos, size, Fade(ORANGE, alpha));
  DrawRectangleLinesEx({pos.x, pos.y, size.x, size.y}, 2, BLACK);
}
