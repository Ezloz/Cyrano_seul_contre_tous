#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>

#include <set>
#include <string>
#include <vector>

#include "Camera.h"
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
    IN_MENU,
    IN_ANIMATION,
    IN_CINEMATIC
};



// Singleton format taken from refactoring.guru 
class GameApp {

    /**
     * The Singleton's constructor should always be private to prevent direct
     * construction calls with the `new` operator.
     */
private:
    GameApp() {
        globalClock.
        LoadOptions();
    };

    static GameApp* singleton_;

    void LoadOptions();

    std::set<Input> inputs;
    GameState state;
    sf::Clock globalClock;
    sf::Time deltaTime;
    Map activeMap;
    Camera activeCamera;
    UIManager GUI;


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

    std::vector<Input> GetInputs() const {return this->inputs;}
    sf::Time GetDeltaTime() const {return deltaTime;}
    GameState GetGameState() const {return state;}
    void SetGameState(GameState state);
    
    void update(){
        deltaTime = globalClock.restart();
//        .update();
//        .update();
//        .update();
//        .update();

    }

//    void SomeBusinessLogic()

};

