#pragma once

#include <raylib.h>

#include <cmath>
#include <iostream>

struct vec2 {
  float x;
  float y;

  vec2() : x(0), y(0) {}
  vec2(float x, float y) : x(x), y(y) {}

  // Raylib conversions
  vec2(const Vector2& v) : x(v.x), y(v.y) {}
  operator Vector2() const { return {x, y}; }

  // Addition
  vec2 operator+(const vec2& other) const {
    return {x + other.x, y + other.y};
  }
  void operator+=(const vec2& other) {
    x += other.x;
    y += other.y;
  }
  void operator+=(float scalar) {
    x += scalar;
    y += scalar;
  }

  // Subtraction
  vec2 operator-(const vec2& other) const {
    return {x - other.x, y - other.y};
  }
  void operator-=(const vec2& other) {
    x -= other.x;
    y -= other.y;
  }
  void operator-=(float scalar) {
    x -= scalar;
    y -= scalar;
  }

  // Muliplication
  vec2 operator*(float scalar) const {
    return {x * scalar, y * scalar};
  }
  void operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
  }

  // Division
  vec2 operator/(float scalar) const {
    return {x / scalar, y / scalar};
  }
  void operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
  }

  // Equality
  bool operator==(const vec2 other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const vec2 other) const {
    return !(*this == other);
  }

  // Unary
  vec2 operator-() const {
    return {-x, -y};
  }

  // Operations
  float length() const {
    return sqrt(x * x + y * y);
  }

  vec2 norm() const {
    float len = length();
    if (len > 0) return *this / len;
    return {};
  }

  float distance(const vec2& other) const {
    return (*this - other).length();
  }

  float dot(const vec2& other) const {
    return x * other.x + y * other.y;
  }

  float cross(const vec2& other) const {
    return x * other.y - y * other.x;
  }

  friend std::ostream& operator<<(std::ostream& stream, const vec2& v) {
    stream << '(' << v.x << ", " << v.y << ')';
    return stream;
  }
};
