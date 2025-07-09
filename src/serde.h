#pragma once

#include "conf.h"
#include "json.h"
#include "level.h"
#include "vec2.h"

using conf::SIZE, nlohmann::json;

inline void to_json(json& j, const vec2& v) {
  j = nlohmann::json::array({v.x, v.y});
}

inline void from_json(const json& j, vec2& v) {
  j.at(0).get_to(v.x);
  j.at(1).get_to(v.y);
}

inline void from_json(const json& j, Rectangle& r) {
  j.at(0).get_to(r.x);
  j.at(1).get_to(r.y);
  j.at(2).get_to(r.width);
  j.at(3).get_to(r.height);
}

inline void from_json(const json& j, Bounds& b) {
  j.at("min").get_to(b.min);
  j.at("max").get_to(b.max);
}

inline void to_json(json& j, Bounds& b) {
  j = json::object({{"min", b.min}, {"max", b.max}});
}

inline void from_json(const json& j, Circle& c) {
  j.at("pos").get_to(c.pos);
  c.pos *= SIZE;

  if (j["move"]["kind"] == "linear") {
    Bounds bounds = j["move"]["bounds"].template get<Bounds>();
    bounds.min *= SIZE;
    bounds.max *= SIZE;
    c.move = std::make_shared<Linear>(
      j["move"]["dir"].template get<vec2>().norm(),
      j["move"]["speed"].template get<float>(),
      bounds
    );
  }
}

inline void to_json(json& j, const Circle& c) {
  j = {{"pos", c.pos / SIZE}, {"move", json::object()}};
  switch (c.move->kind) {
    case Move::Linear: {
      j["move"]["kind"] = "linear";
      Linear* linear = (Linear*)(c.move.get());
      j["move"]["dir"] = linear->dir;
      j["move"]["speed"] = linear->speed;
      j["move"]["bounds"] = {
        {"min", linear->bounds.min / SIZE},
        {"max", linear->bounds.max / SIZE}
      };
    } break;
    default: break;
  }
}
