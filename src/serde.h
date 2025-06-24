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

inline void from_json(const json& j, Circle& c) {
  j.at("pos").get_to(c.pos);
  c.pos *= SIZE;

  if (j["move"]["kind"] == "linear") {
    Bounds bounds = j["move"]["bounds"].template get<Bounds>();
    bounds.min *= SIZE;
    bounds.max *= SIZE;
    c.move = std::make_unique<Linear>(
      j["move"]["dir"].template get<vec2>().norm(),
      j["move"]["speed"].template get<float>(),
      bounds
    );
  }
}
