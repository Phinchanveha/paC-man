#define GAME_CPP
#include <windows.h>											// Header File For Windows
#include <stdio.h>												// Header File For Standard Input / Output
#include <stdarg.h>												// Header File For Variable Argument Routines
#include <math.h>												// Header File For Math Operations
#include <gl\gl.h>												// Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include "glut.h"
#include "baseTypes.h"
#include "openglframework.h"	
#include "gamedefs.h"
#include "collInfo.h"
#include "object.h"
#include "ball.h"
#include "field.h"
#include "random.h"
#include "gameObjects.h"
#include "openGLStuff.h"
#include "game.h"
#include "StateManager.h"
#include "BallManager.h"
#include "FieldManager.h"
#include "InputManager.h"
#include "SpriteDemoManager.h"

#include <map>
#include "..\\GameCode\\Utilities.h"
#include "..\\GameCode\\Tile.h"
#include "..\\GameCode\\Character.h"
#include "..\\GameCode\\Pacman.h"
#include "..\\GameCode\\Ghost.h"
#include "..\\GameCode\\GhostRed.h"
#include "..\\GameCode\\GhostPink.h"
#include "..\\GameCode\\GhostBlue.h"
#include "..\\GameCode\\GhostOrange.h"
#include "..\\GameCode\\BoardManager.h"
#include "..\\GameCode\\CharacterManager.h"

#include <queue>
#include "..\\GameCode\\EventManager.h"

#include "..\\GameCode\\TimerManager.h"

#include "..\\GameCode\\FruitManager.h"

#include "..\\GameCode\\UIManager.h"

#include "..\\GameCode\\StatsManager.h"


// Declarations
const char8_t CGame::mGameTitle[]="paC++man";
CGame* CGame::sInstance=NULL;
BOOL Initialize (GL_Window* window, Keys* keys)					// Any OpenGL Initialization Goes Here
{
	initOpenGLDrawing(window,keys,0.0f, 0.0f, 0.0f);
	CGame::CreateInstance();
	CGame::GetInstance()->init();
	return TRUE;						
}

void CGame::init()
{
    StateManagerC::CreateInstance();
    StateManagerC::GetInstance()->setApplicationState(StateManagerC::IN_MENU);

    StatsManager::Instance()->init();
    UIManager::Instance()->init();

    EventManager::Instance()->init();
    TimerManager::Instance()->init();
    BoardManager::Instance()->init();
    FruitManager::Instance()->init();
    CharacterManager::Instance()->init();

    // start the game on the game over screen
    EventManager::ConstInstace()->postEvent(EventManager::GameEvent::GAME_OVER);
}
void CGame::UpdateFrame(DWORD milliseconds)			
{
    keyProcess();

    switch(StateManagerC::GetInstance()->getApplicationState())
    {
        case StateManagerC::IN_MENU:
            EventManager::Instance()->update();                             // calls events in queue
            UIManager::Instance()->update();                                // listens for start game input
            break;

        case StateManagerC::IN_GAME:
            EventManager::Instance()->update();                             // calls events in queue
            TimerManager::Instance()->updateTimers(milliseconds);           // updates timers for time-delayed events
            BoardManager::Instance()->updateBoard(milliseconds);            // updates tiles and pellets
            FruitManager::Instance()->updateFruit(milliseconds);            // updates fruit
            CharacterManager::Instance()->updateCharacters(milliseconds);   // updates moving characters
            break;

        default:
            break;
    }
}

void CGame::DrawScene(void)											
{
	startOpenGLDrawing();

    switch(StateManagerC::GetInstance()->getApplicationState())
    {
        case StateManagerC::IN_MENU:
            UIManager::Instance()->draw();
            break;

        case StateManagerC::IN_GAME:
            BoardManager::Instance()->drawBoard();
            FruitManager::Instance()->drawFruit();
            CharacterManager::Instance()->drawCharacters();
            UIManager::Instance()->draw();
            break;

        default:
            break;
    }
}


CGame *CGame::CreateInstance()
{
	sInstance = new CGame();
	return sInstance;
}
void CGame::shutdown()
{
    StateManagerC::GetInstance()->shutdown();

    StatsManager::Instance()->shutdown();
    UIManager::Instance()->shutdown();
    EventManager::Instance()->shutdown();

    TimerManager::Instance()->shutdown();
    BoardManager::Instance()->shutdown();
    FruitManager::Instance()->shutdown();
    CharacterManager::Instance()->shutdown();
}
void CGame::DestroyGame(void)
{
    delete StateManagerC::GetInstance();

    delete StatsManager::Instance();
    delete UIManager::Instance();
    delete EventManager::Instance();

    delete TimerManager::Instance();
    delete BoardManager::Instance();
    delete FruitManager::Instance();
    delete CharacterManager::Instance();
}