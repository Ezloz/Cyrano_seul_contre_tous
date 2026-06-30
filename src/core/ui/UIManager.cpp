#include "ui/UIManager.h"

constexpr GameState DEFAULT_STATE = GameState::IN_MENU;

void updateTextSize(tgui::BackendGui& gui)
{
    // Update the text size of all widgets in the gui, based on the current window height
    const float windowHeight = gui.getView().getRect().height;
    gui.setTextSize(static_cast<unsigned int>(0.07f * windowHeight)); // 7% of height
}

GameState UIManager::ProcessInputs(std::set<Input> inputs, std::set<Input> justPressedInputs, std::set<Input> inputsRelease, sf::Time deltaTime){    
    GameState state = this->move(inputs, justPressedInputs, inputsRelease, deltaTime);
    return state;
}


void UIManager::Unload(){
    gui.removeAllWidgets();
}

void UIManager::LoadCharacterStatsMenu(const Character* character){ // NOT WORKING and I don't know why T-T

    if (character == nullptr)
        return;

    auto panel = tgui::ScrollablePanel::create();
    panel->setSize({"100%", "100%"});
    panel->setPosition({"0%", "0%"});
    panel->getRenderer()->setBackgroundColor(tgui::Color(20, 20, 20, 200));

    float y = 0.f;
    const float step = 8.f;

    auto addLine = [&](const std::string& text)
    {
        auto label = tgui::Label::create(text);
        label->setPosition(5, y);
        panel->add(label);
        y += step;
    };

    const auto& s = character->getStats();

    addLine("Character Type: " + character->getType());

    addLine("Life: " + std::to_string(s.life));
    addLine("Max Life: " + std::to_string(s.maxLife));
    addLine("Speed: " + std::to_string(s.speed));
    addLine("Charisma: " + std::to_string(s.charisma));
    addLine("Power: " + std::to_string(s.power));
    addLine("Luck: " + std::to_string(s.luck));
    addLine("Range: " + std::to_string(s.range));

    addLine("Effects:");
    if (character->getEffectsIds().empty())
        addLine("  None");
    else
        for (const auto& e : character->getEffectsIds())
            addLine("  - " + e);

    addLine("Equipment:");
    if (character->getEquipementIds().empty())
        addLine("  None");
    else
        for (const auto& e : character->getEquipementIds())
            addLine("  - " + e);


    printf("Error here :");
    this->gui.add(panel, "StatsPanel");
    printf("No error");
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
        this->current_name = "MainMenu";

        return true;
    }
    catch (const tgui::Exception& e)
    {
        std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
        return false;
    }
}


GameState UIManager::move(std::set<Input> inputs, std::set<Input> justPressedInputs, 
                          std::set<Input> inputsRelease, sf::Time deltaTime){


    auto button = this->gui.getFocusedLeaf();

    if (!button){
        printf("no button");
        if (justPressedInputs.contains(Input::CANCEL)){
            return GameState::IN_GAME;
        }        
        return DEFAULT_STATE;
    }


    if (justPressedInputs.contains(Input::CONFIRM)){
        if (button->getWidgetName() == "ButtonStart"){
            Unload();
            this->current_name = "None";
            return GameState::IN_GAME;
        }
    }

    if (justPressedInputs.contains(Input::UP)){
        button->setFocused(false);
        button->getNavigationUp()->setFocused(true);
    }
    if (justPressedInputs.contains(Input::DOWN)){
        button->setFocused(false);
        button->getNavigationDown()->setFocused(true);
    }

    return DEFAULT_STATE;
}


