
#define EVENT_MANAGER_CPP

#include <assert.h>
#include <windows.h>											// Header File For Windows
#include <stdio.h>												// Header File For Standard Input / Output
#include <stdarg.h>												// Header File For Variable Argument Routines
#include <math.h>												// Header File For Math Operations
#include <gl\gl.h>												// Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include "glut.h"
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\openglframework.h"	
#include "..\\Framework\\gamedefs.h"
#include "..\\Framework\\stateManager.h"

#include <queue>
#include "EventManager.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "Character.h"
#include "Pacman.h"
#include "Ghost.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "BoardManager.h"
#include "CharacterManager.h"

#include "TimerManager.h"

#include "FruitManager.h"

#include "UIManager.h"

#include "StatsManager.h"


// audio filepaths
#define START_MUSIC         "Assets\\Audio\\pacman_beginning.wav"
#define EXTRA_LIFE_SOUND    "Assets\\Audio\\pacman_extrapac.wav"
#define PELLET_SOUND        "Assets\\Audio\\pacman_chomp.wav"
#define FRUIT_SOUND         "Assets\\Audio\\pacman_eatfruit.wav"
#define PACMAN_DEATH        "Assets\\Audio\\pacman_death.wav"
#define GHOST_DEATH         "Assets\\Audio\\pacman_eatghost.wav"


EventManager* EventManager::_instance = nullptr;

EventManager::EventManager() :
    mEventQueue(new EventQueue()),
    mSoundQueue(new SoundQueue())
{
    // managers should only call on other managers for const getter functions
    // they should post events for things that would alter other managers
}

//-----------------------------------------------------------------------------------------

EventManager* EventManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new EventManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const EventManager* EventManager::ConstInstace()
{
    if(_instance == nullptr)
    {
        _instance = new EventManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void EventManager::init()
{
}

//-----------------------------------------------------------------------------------------

void EventManager::reset()
{
    // clear event queues of contents
    while(!mEventQueue->empty())
    {
        mEventQueue->pop();
    }
    while(!mSoundQueue->empty())
    {
        mSoundQueue->pop();
    }
}

//-----------------------------------------------------------------------------------------

void EventManager::update()
{
    handleSoundsEventQueue();   // plays enqueued game sounds
    handleGameEventQueue();     // handles enqueued events by calling other managers
}

//-----------------------------------------------------------------------------------------

void EventManager::handleGameEventQueue()
{
    // as long as there are events in the queue, pop them out and handle them
    EventManager::GameEvent eventType;
    while(!mEventQueue->empty())
    {
        eventType = mEventQueue->front();
        mEventQueue->pop();

        // call handler funcitons on other managers based on the event type
        switch(eventType)
        {
            // game flow -- resetting
            case EventManager::GameEvent::START_FROM_MENU:
                StateManagerC::GetInstance()->setApplicationState(StateManagerC::ApplicationState::IN_GAME);
                StatsManager::Instance()->reset();
                UIManager::Instance()->reset();
                TimerManager::Instance()->reset();
                BoardManager::Instance()->reset();
                FruitManager::Instance()->reset();
                CharacterManager::Instance()->reset();
                reset();
                TimerManager::Instance()->activateStartTimerFromMenu();
                break;
            case EventManager::GameEvent::START_FROM_GAME_WIN:
                StatsManager::Instance()->nextLevel();
                TimerManager::Instance()->reset();
                BoardManager::Instance()->reset();
                FruitManager::Instance()->nextLevel();
                CharacterManager::Instance()->reset();
                reset();
                TimerManager::Instance()->activateStartTimerFromWin();
                break;
            case EventManager::GameEvent::START_FROM_GAME_LOSE:
                CharacterManager::Instance()->resetFromDeath();
                FruitManager::Instance()->resetFromDeath();
                TimerManager::Instance()->reset();
                reset();
                TimerManager::Instance()->activateStartTimerFromLoss();
                break;

            // game flow -- ending game
            case EventManager::GameEvent::LEVEL_WIN:
                FruitManager::Instance()->pauseFruit();
                CharacterManager::Instance()->pauseCharacters();
                TimerManager::Instance()->activateEndGameTimerWin();
                break;
            case EventManager::GameEvent::LEVEL_LOSE:
                FruitManager::Instance()->pauseFruit();
                CharacterManager::Instance()->pauseCharacters();
                TimerManager::Instance()->activateEndGameTimerLoss();
                break;

            // ui events
            case EventManager::GameEvent::GAME_OVER:
                StateManagerC::GetInstance()->setApplicationState(StateManagerC::ApplicationState::IN_MENU);
                UIManager::Instance()->setUIState(GameUtil::UIState::UI_MENU);
                reset();
                break;
            case EventManager::GameEvent::UI_ENTER_PREGAME_2:
                UIManager::Instance()->setUIState(GameUtil::UIState::UI_PREGAME_2);
                break;
            case EventManager::GameEvent::UI_ENTER_GAME:
                UIManager::Instance()->setUIState(GameUtil::UIState::UI_GAME);
                break;

            // pause or unpause events
            case EventManager::GameEvent::PAUSE:
                FruitManager::Instance()->pauseFruit();
                CharacterManager::Instance()->pauseCharacters();
                break;
            case EventManager::GameEvent::UNPAUSE:
                FruitManager::Instance()->unpauseFruit();
                CharacterManager::Instance()->unpauseCharacters();
                break;

            // visibility events
            case EventManager::GameEvent::CHARACTERS_VISIBLE:
                CharacterManager::ConstInstance()->makeCharactersVisible();
                break;
            case EventManager::GameEvent::GHOSTS_INVISIBLE:
                CharacterManager::ConstInstance()->makeGhostsInvisible();
                break;

            // spawn ghost events
            case EventManager::GameEvent::SPAWN_GHOST_RED:
                CharacterManager::ConstInstance()->spawnGhost(GameUtil::CharacterID::GHOST_RED);
                break;
            case EventManager::GameEvent::SPAWN_GHOST_PINK:
                CharacterManager::ConstInstance()->spawnGhost(GameUtil::CharacterID::GHOST_PINK);
                break;
            case EventManager::GameEvent::SPAWN_GHOST_BLUE:
                CharacterManager::ConstInstance()->spawnGhost(GameUtil::CharacterID::GHOST_BLUE);
                break;
            case EventManager::GameEvent::SPAWN_GHOST_ORANGE:
                CharacterManager::ConstInstance()->spawnGhost(GameUtil::CharacterID::GHOST_ORANGE);
                break;

            // ghost events
            case EventManager::GameEvent::SCARE_GHOSTS:
                CharacterManager::ConstInstance()->scareGhosts();
                break;
            case EventManager::GameEvent::CRUISE_ELROY:
                CharacterManager::ConstInstance()->activateCruiseElroy();
                break;
            case EventManager::GameEvent::GHOST_KILLED:
                TimerManager::Instance()->activateDeadGhostTimer();
                StatsManager::Instance()->ghostKilled();
                UIManager::Instance()->updateScoreStrings();
                break;

            // fruit events
            case EventManager::GameEvent::ACTIVATE_FRUIT:
                FruitManager::Instance()->activateFruit();
                break;

            // animation events
            case EventManager::GameEvent::PLAY_PACMAN_DEATH_ANIM:
                CharacterManager::ConstInstance()->playPacmanDeathAnimation();
                break;
            case EventManager::GameEvent::PLAY_TILE_FLASH:
                BoardManager::ConstInstace()->flashTiles();
                break;

            // stats events
            case EventManager::GameEvent::CONSUME_LIFE:
                StatsManager::Instance()->consumeLife();
                break;
            case EventManager::GameEvent::POINTS_PELLET:
                StatsManager::Instance()->pelletCollected();
                UIManager::Instance()->updateScoreStrings();
                break;
            case EventManager::GameEvent::POINTS_POWER_PELLET:
                StatsManager::Instance()->powerPelletCollected();
                UIManager::Instance()->updateScoreStrings();
                break;
            case EventManager::GameEvent::POINTS_KILL_BONUS:
                StatsManager::Instance()->ghostKillBonusPoints();
                UIManager::Instance()->updateScoreStrings();
                break;
            case EventManager::GameEvent::POINTS_FRUIT:
                StatsManager::Instance()->fruitCollected();
                UIManager::Instance()->updateScoreStrings();
                break;

            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void EventManager::handleSoundsEventQueue()
{
    // while sounds are enqueued, pop and handle sound events
    EventManager::SoundEvent eventType;
    while(!mSoundQueue->empty())
    {
        eventType = mSoundQueue->front();
        mSoundQueue->pop();

        switch(eventType)
        {
            case EventManager::SoundEvent::SOUND_START:
                PlaySound(START_MUSIC, NULL, SND_FILENAME | SND_ASYNC);
                break;
            case EventManager::SoundEvent::SOUND_EXTRA_LIFE:
                PlaySound(EXTRA_LIFE_SOUND, NULL, SND_FILENAME | SND_ASYNC);
                break;
            case EventManager::SoundEvent::SOUND_PELLET:
                PlaySound(PELLET_SOUND, NULL, SND_FILENAME | SND_ASYNC);
                break;
            case EventManager::SoundEvent::SOUND_FRUIT:
                PlaySound(FRUIT_SOUND, NULL, SND_FILENAME | SND_ASYNC);
                break;
            case EventManager::SoundEvent::SOUND_PACMAN_DEATH:
                PlaySound(PACMAN_DEATH, NULL, SND_FILENAME | SND_ASYNC);
                break;
            case EventManager::SoundEvent::SOUND_GHOST_DEATH:
                PlaySound(GHOST_DEATH, NULL, SND_FILENAME | SND_ASYNC);
                break;
            default:
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void EventManager::shutdown()
{
    delete mEventQueue;
    delete mSoundQueue;
}

//-----------------------------------------------------------------------------------------

void EventManager::postEvent(EventManager::GameEvent eventType) const
{
    mEventQueue->push(eventType);
}

//-----------------------------------------------------------------------------------------

void EventManager::postSoundEvent(EventManager::SoundEvent eventType) const
{
    mSoundQueue->push(eventType);
}
