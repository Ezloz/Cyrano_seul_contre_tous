#include "ui/UIManager.h"

constexpr GameState DEFAULT_STATE = GameState::IN_MENU;

void updateTextSize(tgui::BackendGui& gui)
{
    // Update the text size of all widgets in the gui, based on the current window height
    const float windowHeight = gui.getView().getRect().height;
    gui.setTextSize(static_cast<unsigned int>(0.07f * windowHeight)); // 7% of height
}

GameState UIManager::ProcessInputs(std::set<Input> inputs, std::set<Input> inputsRelease, sf::Time deltaTime){    
    GameState state = this->move(inputs, inputsRelease, deltaTime);
    return state;
}
    

//taken and modified from TGUI website
bool UIManager::LoadGUI(const std::string& pathname)
{
    try
    {
        this->gui.loadWidgetsFromFile(pathname);
        auto button = gui.get<tgui::Button>("ButtonStart");

        if (button) {
            button->setFocused(true);
        }

        return true;
    }
    catch (const tgui::Exception& e)
    {
        std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
        return false;
    }
}


std::array<bool, 2> buttonPressed = {false, false}; // this is very bad and a result of the way inputs are given.
GameState UIManager::move(std::set<Input> inputs, std::set<Input> inputsRelease, sf::Time deltaTime){
    auto button = this->gui.getFocusedLeaf();

    if (inputs.find(Input::CONFIRM) != inputs.end()){
        if (button->getWidgetName() == "ButtonStart"){
            return GameState::IN_GAME;
        }
    }

    if (inputsRelease.find(Input::UP) != inputsRelease.end()){
        buttonPressed[0] = false;
    }
    if (inputsRelease.find(Input::DOWN) != inputsRelease.end()){
        buttonPressed[1] = false;
    }
    if (inputs.find(Input::UP) != inputs.end() && !buttonPressed[0]){
        buttonPressed[0] = true;
        button->setFocused(false);
        button->getNavigationUp()->setFocused(true);
    }
    if (inputs.find(Input::DOWN) != inputs.end() && !buttonPressed[1]){
        buttonPressed[1] = true;
        button->setFocused(false);
        button->getNavigationDown()->setFocused(true);
    }

    return DEFAULT_STATE;
}


