#pragma once

#include <SFML/System/Time.hpp>

#include <string>
#include <vector>


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


// Singleton format taken from refactoring.guru 
class GameApp {

    /**
     * The Singleton's constructor should always be private to prevent direct
     * construction calls with the `new` operator.
     */
private:
    GameApp() {LoadOptions();};

    static GameApp* singleton_;

    void LoadOptions();

    std::vector<Input> inputs;

    sf::Time deltaTime;

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
    static GameApp *GetInstance();

    std::vector<Input> GetInputs() const {return this->inputs;};
    sf::Time GetDeltaTime() const {return deltaTime;};

//    void SomeBusinessLogic()

};

