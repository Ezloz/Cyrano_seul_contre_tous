#include "entities/CharacterFactory.h"

#include "entities/Character.h"
#include "entities/Cyrano.h"
#include "entities/Soldier.h"

#include <stdexcept>

std::unordered_map<std::string, CharacterFactory::Creator> &
CharacterFactory::registry() {
  static std::unordered_map<std::string, Creator> instance;
  return instance;
}

void CharacterFactory::registerType(const std::string &type, Creator creator) {
  registry()[type] = std::move(creator);
}

std::unique_ptr<Character> CharacterFactory::create(const json &charJson) {
  const std::string type = charJson.at("type").get<std::string>();
  auto it = registry().find(type);
  if (it == registry().end()) {
    throw std::runtime_error("Unknown character type: " + type);
  }
  return it->second(charJson);
}

bool CharacterFactory::isRegistered(const std::string &type) {
  return registry().find(type) != registry().end();
}

void CharacterFactory::registerBuiltins() {
  // Explicitly referencing the concrete creators here also forces their
  // translation units to be linked from the static library, so registration
  // actually happens (static self-registration would be stripped otherwise).
  registerType("Cyrano", &Cyrano::create);
  registerType("Soldier", &Soldier::create);
}
