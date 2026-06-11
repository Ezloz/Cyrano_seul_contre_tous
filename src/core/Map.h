#include "MapLayer.h"
#include "Character.h"
#include "TurnQueue.h"

class Map {
    private:
        tmx::Map tmxMap;

    public:
        explicit Map(const std::string& name) {tmxMap.load(name);};
        tmx::Map* GetTMXMap() {return &tmxMap;};    
    
    
    
};


