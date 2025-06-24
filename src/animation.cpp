#include "animation.h"

FadeAnimation::FadeAnimation(float duration, float steps) : frame_delay(duration / steps) {
  for (int i = steps; i >= 0; i--) {
    frames.push_back(i / (steps - 1));
  }
}

void FadeAnimation::reset() {
  index = 0;
  timer = 0;
  done = false;
}

float FadeAnimation::current_frame() const {
  return frames[index];
}

void FadeAnimation::update(float dt) {
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
