#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <set>
#include <map>
#include <string>
#include <vector>
#include <variant>

#include "Map.h"
#include "UIManager.h"


enum class Input{
    UP,
    DOWN,
    LEFT,
    RIGHT,
    CONFIRM,
    CANCEL,
    MENU,
    NB_INPUTS
};

enum class GameState{
    IN_GAME,
    IN_GAMEMENU,
    IN_MENU,
    IN_ANIMATION,
    IN_CINEMATIC
};

using RawInput = std::variant<sf::Event::KeyPressed, sf::Event::JoystickButtonPressed>;

// Singleton format taken from refactoring.guru 
class GameApp {
    
    /**
     * The Singleton's constructor should always be private to prevent direct
     * construction calls with the `new` operator.
     */
    private:
    GameApp() {
        LoadOptions();
    };
    
    static GameApp* singleton_;
    
    void SaveOptions();    
    void LoadOptions();

    std::map<RawInput, Input> rawToInput;

    std::set<Input> inputs;
    GameState state;
    sf::Clock globalClock;
    sf::Time deltaTime;
    Map activeMap;
//    Camera activeCamera;
    UIManager gui;


public:

    /**
     * Singletons should not be cloneable.
     */
    GameApp(GameApp &other) = delete;
    /**
     * Singletons should not be assignable.
     */
    void operator=(const GameApp &) = delete;
    /**
     * This is the static method that controls the access to the singleton
     * instance. On the first run, it creates a singleton object and places it
     * into the static field. On subsequent runs, it returns the client existing
     * object stored in the static field.
     */
    static GameApp* GetInstance();

    sf::Time GetDeltaTime() const {return deltaTime;}
    GameState GetGameState() const {return state;}
    void SetGameState(GameState state);
    std::set<Input> GetInputs() const {return inputs;}
    Map GetActiveMap() const {return activeMap;}

    void HandleRawInput(RawInput rinput);

    void ProcessInputs(){
        switch (this->state){
            case (GameState::IN_GAME):
//                activeCamera.move(this->inputs);
                break;
        }
    }
    
    void SaveGame();
    void LoadGame();


    void Update(){
        deltaTime = globalClock.restart();
//        .update();
//        .update();
//        .update();
//        .update();

    }

//    void SomeBusinessLogic()

    void Quit();
};

