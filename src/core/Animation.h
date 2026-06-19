#pragma once

#include <SFML/System/Time.hpp>

#include <cstdint>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

inline constexpr const char *DATASET = "resources/dataset.json";
inline std::unordered_map<std::string, std::string> textureDataset;

inline void parseTextureDataset() {
  std::ifstream f(DATASET);
  if (!f)
    throw std::runtime_error(std::string("Cannot open dataset: ") + DATASET);
  json data = json::parse(f);
  for (auto it = data.at("texture").begin(); it != data.at("texture").end();
       ++it) {
    textureDataset[it.key()] = it.value();
  }
}

struct Frame {
  int x, y, w, h;
  sf::Time duration;
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
  AnimationTemplate(const std::string &textureId) {
    m_textureId = textureId;
    auto pathIt = textureDataset.find(textureId);
    if (pathIt == textureDataset.end())
      throw std::runtime_error("Texture inconnue dans le dataset: " +
                               textureId);

    std::ifstream f(pathIt->second);
    if (!f)
      throw std::runtime_error("Impossible d'ouvrir: " + pathIt->second);
    json data = json::parse(f);

    m_defaultState = data.value("defaultState", "idle");

    for (auto &[stateName, stateJson] : data.at("states").items()) {
      Animation anim;

      std::optional<sf::Time> defaultDuration;
      if (stateJson.contains("defaultFrameDuration"))
        defaultDuration =
            sf::seconds(stateJson["defaultFrameDuration"].get<float>());

      std::optional<int> defW, defH;
      if (stateJson.contains("defaultFrameSize")) {
        defW = stateJson["defaultFrameSize"].at("w").get<int>();
        defH = stateJson["defaultFrameSize"].at("h").get<int>();
      }

      const auto &frames = stateJson.at("frames");
      if (frames.empty())
        throw std::runtime_error("State '" + stateName + "' sans frames");

      for (const auto &fJson : frames) {
        Frame frame;
        frame.x = fJson.at("x").get<int>();
        frame.y = fJson.at("y").get<int>();

        if (fJson.contains("w") && fJson.contains("h")) {
          frame.w = fJson["w"].get<int>();
          frame.h = fJson["h"].get<int>();
        } else if (defW && defH) {
          frame.w = *defW;
          frame.h = *defH;
        } else {
          throw std::runtime_error("State '" + stateName +
                                   "': frame sans taille et pas de "
                                   "defaultFrameSize");
        }

        if (fJson.contains("duration")) {
          float secs = fJson["duration"].get<float>();
          if (secs <= 0.f)
            throw std::runtime_error("State'" + stateName +
                                     "': durée de frame <= 0");
          frame.duration = sf::seconds(secs);
        } else if (defaultDuration) {
          frame.duration = *defaultDuration;
        } else {
          throw std::runtime_error("State '" + stateName +
                                   "': frame sans durée et pas de "
                                   "defaultFrameDuration");
        }

        anim.frames.push_back(frame);
      }
      m_animations.emplace(stateName, std::move(anim));
    }
  }

  const std::string &textureId() const { return m_textureId; }
  const std::string &defaultState() const { return m_defaultState; }

  const Animation &find(const std::string &s) const {
    auto it = m_animations.find(s);
    if (it == m_animations.end())
      throw std::out_of_range("L'état '" + s + "' n'existe pas");
    return it->second;
  }
};
