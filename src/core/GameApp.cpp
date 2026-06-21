#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "GameApp.h"

void GameApp::SaveOptions() {}

void GameApp::LoadOptions() {
  this->state = GameState::IN_GAME;
  this->activeMap = std::make_unique<Map>("resources/little_test_20.tmx");

  std::ifstream is("resources/options.txt");
  std::ofstream os("resources/options.txt");
  std::string ligne;

  this->keyboardToInput = {{sf::Keyboard::Key::Up, Input::UP},
                           {sf::Keyboard::Key::Down, Input::DOWN},
                           {sf::Keyboard::Key::Left, Input::LEFT},
                           {sf::Keyboard::Key::Right, Input::RIGHT},
                           {sf::Keyboard::Key::W, Input::CONFIRM},
                           {sf::Keyboard::Key::X, Input::CANCEL},
                           {sf::Keyboard::Key::C, Input::MENU}};
}


void GameApp::SetGameState(GameState s) { this->state = s; }

void GameApp::transformRawInputToInput(RawInput rinput) {
  if (std::holds_alternative<const sf::Event::KeyPressed *>(rinput)) {
    auto input = this->keyboardToInput.find(
        std::get<const sf::Event::KeyPressed *>(rinput)->code);
    if (input != this->keyboardToInput.end()) {
      this->inputs.emplace(input->second);
    }
  } else if (std::holds_alternative<const sf::Event::KeyReleased *>(rinput)) {
    auto input = this->keyboardToInput.find(
        std::get<const sf::Event::KeyReleased *>(rinput)->code);
    if (input != this->keyboardToInput.end()) {
      this->inputs.erase(input->second);
      this->releasedInputs.emplace(input->second);
    }
  }
}

void GameApp::SaveGame() {}

void GameApp::LoadGame() {}

void GameApp::Quit() { this->SaveOptions(); }

