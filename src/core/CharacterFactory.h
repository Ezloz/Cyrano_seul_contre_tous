#pragma once

#include "Save.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class Character;

class CharacterFactory {
public:
  using Creator = std::function<std::unique_ptr<Character>(const json &)>;

  static void registerType(const std::string &type, Creator creator);

  // Builds a character from a json entry; throws if its "type" is unknown.
  static std::unique_ptr<Character> create(const json &charJson);

  // True if `type` has a registered creator.
  static bool isRegistered(const std::string &type);

  // Registers every built-in concrete type. Idempotent; call before create().
  static void registerBuiltins();

private:
  static std::unordered_map<std::string, Creator> &registry();
};
