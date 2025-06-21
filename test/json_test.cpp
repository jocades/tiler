#include "json.hpp"

#include <raylib.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

using json = nlohmann::json;

struct User {
  std::string name;
  int age;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(User, name, age)

struct vec2 {
  float x, y;
};

void to_json(json& j, const vec2& v) {
  j = json::array({v.x, v.y});
}

void from_json(const json& j, vec2& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
}

// void to_json(json& j, const User& u) {
//   j = json{{"name", u.name}, {"age", u.age}};
// }
//
// void from_json(const json& j, User& u) {
//   j.at("name").get_to(u.name);
//   j.at("age").get_to(u.age);
// }

int main() {
  vec2 v = {2.5f, 3.2f};
  json j = v;

  std::cout << j << '\n';
  auto v1 = j.template get<vec2>();

  std::ofstream o("test2.json");
  if (!o.is_open()) return 1;
  o << j.dump();
  o.close();

  std::cout << v1.x << v1.y << '\n';

  std::ifstream f("test.json");
  if (!f.is_open()) return 1;
  json j1 = json::parse(f);
  // std::cout << j1 << '\n';
  // User u3 = j1.template get<User>();
}
