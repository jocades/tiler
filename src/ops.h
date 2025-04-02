#pragma once

#include <iostream>

#include "raylib.h"

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Vector2& operator+=(Vector2& lhs, const Vector2& rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
}

inline Vector2 operator+(const Vector2& lhs, float scalar) {
  return {lhs.x + scalar, lhs.y + scalar};
}

inline Vector2 operator+=(Vector2& lhs, float scalar) {
  lhs.x += scalar;
  lhs.y += scalar;
  return lhs;
}

inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline Vector2& operator-=(Vector2& lhs, const Vector2& rhs) {
  lhs.x -= rhs.x;
  lhs.y -= rhs.y;
  return lhs;
}

inline Vector2 operator*(const Vector2& lhs, float scalar) {
  return {lhs.x * scalar, lhs.y * scalar};
}

inline Vector2& operator*=(Vector2& lhs, float scalar) {
  lhs.x *= scalar;
  lhs.y *= scalar;
  return lhs;
}

inline Vector2 operator/(const Vector2& lhs, float scalar) {
  return {lhs.x / scalar, lhs.y / scalar};
}

inline Vector2& operator/=(Vector2& lhs, float scalar) {
  lhs.x /= scalar;
  lhs.y /= scalar;
  return lhs;
}

inline void normalize(Vector2& v) {
  float length = sqrtf((v.x * v.x) + (v.y * v.y));
  if (length > 0) v /= length;
}

inline std::ostream& operator<<(std::ostream& stream, const Vector2& pos) {
  stream << "(" << pos.x << ", " << pos.y << ")";
  return stream;
}
