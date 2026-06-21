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

void login(tgui::EditBox::Ptr username, tgui::EditBox::Ptr password) {
  std::cout << "Username: " << username->getText() << std::endl;
  std::cout << "Password: " << password->getText() << std::endl;
}

void updateTextSize(tgui::BackendGui &gui) {
  // Update the text size of all widgets in the gui, based on the current window
  // height
  const float windowHeight = gui.getView().getRect().height;
  gui.setTextSize(
      static_cast<unsigned int>(0.07f * windowHeight)); // 7% of height
}

void loadWidgets(tgui::BackendGui &gui) {
  // Specify an initial text size instead of using the default value
  updateTextSize(gui);

  // We want the text size to be updated when the window is resized
  gui.onViewChange([&gui] { updateTextSize(gui); });

  // Create the background image
  // The picture is of type tgui::Picture::Ptr which is actually just a typedef
  // for std::shared_widget<tgui::Picture> The picture will fit the entire
  // window and will scale with it
  //    auto picture = tgui::Picture::create("../resources/cursor.png");
  //    picture->setSize({"100%", "100%"});
  //    gui.add(picture);

  // Create the username edit box
  // Similar to the picture, we set a relative position and size
  // In case it isn't obvious, the default text is the text that is displayed
  // when the edit box is empty
  auto editBoxUsername = tgui::EditBox::create();
  editBoxUsername->setSize({"66.67%", "12.5%"});
  editBoxUsername->setPosition({"16.67%", "16.67%"});
  editBoxUsername->setDefaultText("Username");
  gui.add(editBoxUsername);

  // Create the password edit box
  // We copy the previous edit box here and keep the same size
  auto editBoxPassword = tgui::EditBox::copy(editBoxUsername);
  editBoxPassword->setPosition({"16.67%", "41.6%"});
  editBoxPassword->setPasswordCharacter('*');
  editBoxPassword->setDefaultText("Password");
  gui.add(editBoxPassword);

  // Create the login button
  auto button = tgui::Button::create("Login");
  button->setSize({"50%", "16.67%"});
  button->setPosition({"25%", "70%"});
  gui.add(button);

  // Call the login function when the button is pressed and pass the edit boxes
  // that we created as parameters The "&" in front of "login" can be removed on
  // newer compilers, but is kept here for compatibility with GCC < 8.
  button->onPress(&login, editBoxUsername, editBoxPassword);
}

bool GameApp::LoadGUI(std::string name) {
  try {
    loadWidgets(this->gui);
    return true;
  } catch (const tgui::Exception &e) {
    std::cerr << "Failed to load TGUI widgets: " << e.what() << std::endl;
    return false;
  }
}

void jsonDefaultMap(std::string savePath) {
  json defaultJson = {
    "inventory" : [],
    "map" : "castleBlue",
    "playerCharacters" : {
      "Cyrano" : {
        "type" : "Cyrano",
        "statistics" : [
          {"life" : 0}, {"speed" : 0}, {"charisma" : 0}, {"power" : 0},
          {"luck" : 0}, {"range" : 0}
        ],
        "effects" : [],
        "equipped" : []
      }
    }
  };
  writeSave(savePath, saveJson);
}
