#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "UIManager.h"
#include "GameTypes.h"
#include "Map.h"

#include <map>
#include <set>
#include <string>
#include <variant>
#include <vector>

using RawInput =
    std::variant<const sf::Event::KeyPressed *, const sf::Event::KeyReleased *,
                 const sf::Event::JoystickButtonPressed *>;

// Singleton format taken from refactoring.guru
class GameApp {

  /**
   * The Singleton's constructor should always be private to prevent direct
   * construction calls with the `new` operator.
   */
private:
  void SaveOptions();
  void LoadOptions();

  std::map<sf::Keyboard::Key, Input> keyboardToInput;

  std::set<Input> inputs;
  std::set<Input> releasedInputs;
  GameState state;
  sf::Clock globalClock;
  sf::Time deltaTime;
  std::unique_ptr<Map> activeMap;
  UIManager uimanager;

public:
  explicit GameApp(sf::RenderWindow *window) : uimanager{window} { LoadOptions(); }

  sf::Time GetDeltaTime() const { return deltaTime; }
  GameState GetGameState() const { return state; }
  void SetGameState(GameState state);

  std::set<Input> GetInputs() const { return inputs; }
  std::set<Input> GetReleasedInputs() const { return releasedInputs; }
  void ClearReleasedInputs() { releasedInputs.clear(); }

  Map *GetActiveMap() const { return activeMap.get(); }

  void transformRawInputToInput(RawInput rinput);

  void ProcessInputs() {
    GameState nextState = this->state;
    switch (this->state) {
    case GameState::IN_GAME:
      //...
      //nextState = map.ProcessInputs(this->state)
      break;
    case GameState::IN_GAMEMENU:
      //...
      //nextState = uimanager.ProcessInputs(this->state)
      break;
    case GameState::IN_MENU:
      //...
      //nextState = uimanager.ProcessInputs(this->state)
      break;
    case GameState::IN_ANIMATION:
      break;
    case GameState::IN_CINEMATIC:
      break;
    }
    this->state = nextState;
  }

  void SaveGame();
  void LoadGame();
  void LoadGUI(std::string name) {uimanager.LoadGUI(name);};


  void Update() {
    deltaTime = globalClock.restart();
    releasedInputs.clear();
    //        .update();
    //        .update();
    //        .update();
    //        .update();
  }

  void Draw(const sf::RenderTarget &window) {
    //    this->activeMap.draw(window)
    this->uimanager.draw();
    //        .update();
    //        .update();
    //        .update();
    //        .update();
  }

  //    void SomeBusinessLogic()

  void Quit();
};
