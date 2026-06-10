#include <GameApp.h>

GameApp* GameApp::singleton_= nullptr;;

/**
 * Static methods should be defined outside the class.
 */
GameApp* GameApp::GetInstance()
{
    /**
     * This is a safer way to create an instance. instance = new Singleton is
     * dangeruous in case two instance threads wants to access at the same time
     */
    if(singleton_==nullptr){
        singleton_ = new GameApp();
    }
    return singleton_;
}

void GameApp::SetGameState(GameState s){
    this->state = s;
}
