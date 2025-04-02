#include "player.h"

#include "ops.h"

Rectangle Player::rect() const {
  return {
    .x = pos.x,
    .y = pos.y,
    .width = size.x,
    .height = size.y,
  };
}

void Player::input() {
  dir.x = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
  dir.y = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
  normalize(dir);
}

void Player::move(float dt) {
  pos += dir * speed * dt;
}

void Player::update(float dt) {
  input();
  move(dt);
}

void Player::draw() const {
  DrawRectangleV(pos, size, ORANGE);
}
