#pragma once

#include <vector>

class FadeAnimation {
 public:
  float frame_delay;
  int index = 0;
  float timer = 0;
  bool done = false;
  std::vector<float> frames;

  FadeAnimation(float duration, float steps);

  void reset();
  float current_frame() const;
  void update(float dt);
};
