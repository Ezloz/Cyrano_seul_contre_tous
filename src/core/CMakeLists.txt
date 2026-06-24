add_library(lib_core
	app/GameTypes.h
	app/myMain.h app/myMain.cpp
	app/GameApp.h app/GameApp.cpp
	io/Save.h io/Save.cpp
	world/Map.h world/Map.cpp
	world/TurnQueue.h world/TurnQueue.cpp
	graphics/MapLayer.h graphics/MapLayer.cpp
	graphics/Camera.h graphics/Camera.cpp
	graphics/Animation.h
	entities/Character.h entities/Character.cpp
	entities/CharacterFactory.h entities/CharacterFactory.cpp
	entities/CharacterAI.cpp
	entities/PlayerCharacter.h
	entities/NPC.h
	entities/Cyrano.h entities/Cyrano.cpp
	entities/Soldier.h entities/Soldier.cpp
	ui/UIManager.h ui/UIManager.cpp
)

target_include_directories(lib_core PUBLIC ./)
target_link_libraries(lib_core PUBLIC sfml-graphics TGUI::TGUI nlohmann_json::nlohmann_json)
