#include "timer.h"

#include <raylib.h>

Timer::Timer(float duration, bool repeat, bool autostart, std::function<void()> callback)
    : _duration(duration), _repeat(repeat), _callback(callback) {
  if (autostart) start();
}

void Timer::start() {
  _active = true;
  _start_time = GetTime();
}

void Timer::stop() {
  _active = false;
  _start_time = 0;
  if (_repeat) start();
}

void Timer::update() {
  if (_active && GetTime() - _start_time >= _duration) {
    if (_callback) _callback();
    stop();
  }
}
