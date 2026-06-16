add_library(lib_core
	myMain.h myMain.cpp
	GameApp.h GameApp.cpp
	Map.h Map.cpp
	MapLayer.h MapLayer.cpp
  Camera.h Camera.cpp
	Character.h
	UIManager.h
	TurnQueue.h
)

target_include_directories(lib_core PUBLIC ./)
target_link_libraries(lib_core PUBLIC sfml-graphics)
