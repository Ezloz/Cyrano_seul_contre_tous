#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ui/UIManager.h"
#include "app/GameTypes.h"
#include "world/Map.h"

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
  std::set<Input> pressedInputs;
  std::set<Input> releasedInputs;
  GameState state;
  sf::Clock globalClock;
  sf::Time deltaTime;
  std::unique_ptr<Map> activeMap;
  int currentSlot = 1;
  UIManager uimanager;

public:
  explicit GameApp(sf::RenderWindow& window) : uimanager{window} { LoadOptions(); }

  sf::Time GetDeltaTime() const { return deltaTime; }
  GameState GetGameState() const { return state; }
  void SetGameState(GameState state) { this->state = state; }

  std::set<Input> GetInputs() const { return inputs; }
  std::set<Input> GetPressedInputs() const { return pressedInputs; }
  std::set<Input> GetReleasedInputs() const { return releasedInputs; }
  void ClearReleasedInputs() { releasedInputs.clear(); }

  Map *GetActiveMap() const { return activeMap.get(); }

  sf::View GetViewSize();

  void transformRawInputToInput(RawInput rinput);

  void ProcessInputs() {
    GameState nextState = this->state;
      //...
      if (this->state == GameState::IN_GAME)
        nextState = activeMap->ProcessInputs(inputs, pressedInputs, releasedInputs, deltaTime);
      //...
      if (this->state == GameState::IN_MENU)
        nextState = uimanager.ProcessInputs(inputs, pressedInputs, releasedInputs, deltaTime);
      //...
    this->state = nextState;
  }



  void SaveGame();
  void LoadGame();
  
  void LoadGUI(std::string pathname) {uimanager.LoadGUI(pathname);}
  void LoadMap(int slot, const std::string& startMap) {this->currentSlot = slot; this->activeMap = Map::loadMap(slot, startMap);}

  void Start(){
    this->state = GameState::IN_MENU;
    this->LoadGUI("resources/GUI/MainMenu.txt");
    const int slot = 1;
    const std::string startMap = "castleBlue";
    this->LoadMap(slot, startMap);
  }

  void Update() {
    deltaTime = globalClock.restart();
    releasedInputs.clear();
    activeMap->update(deltaTime);
    //        uimanager.update(this->state, deltaTime);
  }

  void Draw(sf::RenderTarget& window) {
    //    this->activeMap.draw(window)
    if (this->state != GameState::IN_MENU){
      window.draw(*(this->activeMap));
    }
    if (this->state != GameState::IN_GAME){
      this->uimanager.draw();
    }
//    window.draw();
    //        .update();
    //        .update();
    //        .update();
    //        .update();
  }

  //    void SomeBusinessLogic()

  void Quit();
};
