add_library(lib_core
	myMain.h myMain.cpp
	MapLayer.h MapLayer.cpp
	Character.h
)

target_include_directories(lib_core PUBLIC ./)
target_link_libraries(lib_core PUBLIC sfml-graphics)
