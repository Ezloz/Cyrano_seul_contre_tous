#pragma once

#include <SFML/System/Time.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct Frame {
  int x, y, w, h;
  sf::Time duration = sf::seconds(0.1f);
};

struct Animation {
  std::vector<Frame> frames;
};

class AnimationTemplate {
private:
  std::string m_textureId;
  std::unordered_map<std::string, Animation> m_animations;
  std::string m_defaultState = "idle";

public:
  AnimationTemplate() = default;
  AnimationTemplate(std::string textureId,
                    std::unordered_map<std::string, Animation> anims,
                    std::string defaultState = "idle")
      : m_textureId(std::move(textureId)), m_animations(std::move(anims)),
        m_defaultState(std::move(defaultState)) {}

  const std::string &textureId() const { return m_textureId; }
  const std::string &defaultState() const { return m_defaultState; }

  const Animation &find(const std::string &s) const {
    auto it = m_animations.find(s);
    if (it == m_animations.end()) {
      throw std::out_of_range("L'état '" + s + "' n'existe pas");
    }
    return it->second;
  }
};
