#include "json.hpp"

#include <raylib.h>

#include <fstream>
#include <iostream>
#include <memory>

using json = nlohmann::json;

struct Move {
  virtual void update() = 0;
};

struct Linear : public Move {
  std::string name;

  Linear(const std::string& name) : name(name) {}

  void update() override { std::cout << name << " subclass\n"; }
};

int main() {
  json data = {
    {"start", {1, 2}},
    {"obstacles",
     {
       {"a", {3, 3}},
       {"b", {5, 6}},
     }},
  };

  Vector2 pos = {10, 20};
  json v;
  v["pos"] = {pos.x, pos.y};

  std::cout << v << '\n';

  // Linear l("Jordi");
  // std::vector<Move*> moves;
  //
  // moves.push_back(&l);

  std::vector<std::unique_ptr<Move>> moves;
  moves.emplace_back(std::make_unique<Linear>("Jordi"));
  moves.push_back(std::make_unique<Linear>("Other"));

  for (const auto& move : moves) {
    move->update();
  }

  // std::ofstream f("test.json");
  // if (!f.is_open()) return 1;
  // f << data.dump();
}
