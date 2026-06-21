add_library(lib_core
	GameTypes.h
	Save.h Save.cpp
	myMain.h myMain.cpp
	GameApp.h GameApp.cpp
	Map.h Map.cpp
	MapLayer.h MapLayer.cpp
  Camera.h Camera.cpp
	Character.h Character.cpp
	CharacterFactory.h CharacterFactory.cpp
	PlayerCharacter.h
	NPC.h
	Cyrano.h Cyrano.cpp
	Soldier.h Soldier.cpp
	UIManager.h UIManager.cpp
	TurnQueue.h TurnQueue.cpp
)

target_include_directories(lib_core PUBLIC ./)
target_link_libraries(lib_core PUBLIC sfml-graphics TGUI::TGUI nlohmann_json::nlohmann_json)
