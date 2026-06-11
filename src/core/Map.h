#include "MapLayer.h"
#include "Character.h"
#include "TurnQueue.h"
// #include "Camera.h"

class Map {
    private:
        tmx::Map tmxMap;
//        Camera activeCamera;


    public:
        explicit Map(const std::string& name) {tmxMap.load(name);};
        tmx::Map* GetTMXMap() {return &tmxMap;};    
    
    
    
};


