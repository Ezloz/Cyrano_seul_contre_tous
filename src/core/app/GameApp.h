#pragma once

#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "ui/UIManager.h"
#include "app/GameTypes.h"
#include "io/Save.h"
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
  std::set<Input> justPressedInputs;
  std::set<Input> justReleasedInputs;
  GameState state;
  sf::Clock globalClock;
  sf::Time deltaTime;
  std::unique_ptr<Map> activeMap;
  inline static int currentSlot = 1;
  UIManager uimanager;

public:
  explicit GameApp(sf::RenderWindow& window) : uimanager{window} { LoadOptions(); }

  // Slot de sauvegarde courant, accessible sans instance (cf. Map::update).
  static int getSlot() { return currentSlot; }

  sf::Time GetDeltaTime() const { return deltaTime; }
  GameState GetGameState() const { return state; }
  void SetGameState(GameState state) { this->state = state; }

  std::set<Input> GetInputs() const { return inputs; }
  std::set<Input> GetPressedInputs() const { return justPressedInputs; }
  std::set<Input> GetReleasedInputs() const { return justReleasedInputs; }
  void ClearJustReleasedInputs() { justReleasedInputs.clear(); }
  void ClearJustPressedInputs() { justPressedInputs.clear(); }

  Map *GetActiveMap() const { return activeMap.get(); }

  sf::View GetViewSize();

  void transformRawInputToInput(RawInput rinput);

  void ProcessInputs() {
    GameState nextState = this->state;
      //...
      if (this->state == GameState::IN_GAME)
        nextState = activeMap->ProcessInputs(inputs, justPressedInputs, justReleasedInputs, deltaTime);
      //...
      if (this->state == GameState::IN_MENU)
        nextState = uimanager.ProcessInputs(inputs, justPressedInputs, justReleasedInputs, deltaTime);
      //...
    this->state = nextState;
  }



  void SaveGame();
  void LoadGame();
  
  void LoadMap(int slot, const std::string& startMap) {this->currentSlot = slot; this->activeMap = Map::loadMap(&uimanager, slot, startMap);}

  void Start(){ //TO REWORK
    this->state = GameState::IN_MENU;
    this->uimanager.LoadGUI("resources/GUI/MainMenu.txt");
    const int slot = 1; 
    const std::string startMap = "castleBlue";
    this->LoadMap(slot, startMap);
  }

  void Update() {
    deltaTime = globalClock.restart();
    justReleasedInputs.clear();
    justPressedInputs.clear();
    if (activeMap){
      activeMap->update(deltaTime);

      // Party anéantie : efface la sauvegarde du slot et passe en GAME_OVER.
      if (state != GameState::GAME_OVER && !activeMap->hasPlayerCharacters()) {
        deleteSave(currentSlot);
//        state = GameState::GAME_OVER; Doesn't work ?
      }
      if (state != GameState::GAME_OVER && !activeMap->hasEnemyCharacters()) {
        deleteSave(currentSlot);
//        state = GameState::GAME_OVER; Doesn't work ?
      }

    }
//    uimanager.update();
  }

  void Draw(sf::RenderTarget& window) {

  if (this->state == GameState::IN_GAME){
    window.draw(*(this->activeMap));
  }
    if (this->state == GameState::IN_MENU){
      this->uimanager.draw();
    }
  }

  //    void SomeBusinessLogic()

  void Quit();
};
