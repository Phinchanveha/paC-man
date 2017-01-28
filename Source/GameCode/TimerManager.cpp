
#define TIMER_MANAGER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include "TimerManager.h"

#include <queue>
#include "EventManager.h"

#include "Utilities.h"
#include "StatsManager.h"


#define START_GAME_PAUSE_DURATION   4000
#define START_GAME_HALF             2000

#define END_GAME_PAUSE_DURATION     3000
#define END_GAME_HALF               1500

#define GHOST_KILL_PAUSE_DURATION   750


TimerManager* TimerManager::_instance = nullptr;

TimerManager::TimerManager()
{
}

//-----------------------------------------------------------------------------------------

TimerManager* TimerManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new TimerManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const TimerManager* TimerManager::ConstInstace()
{
    if(_instance == nullptr)
    {
        _instance = new TimerManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void TimerManager::init()
{
}

//-----------------------------------------------------------------------------------------

void TimerManager::reset()
{
    // reset timer values to 0
    mStartGameTimer = 0;
    mEndGameTimer = 0;
    mDeadGhostTimer = 0;

    // reset boolean values
    mHalfPauseTimePassed = false;
    mWillConsumeLife = false;
    mDidLose = false;
}

//-----------------------------------------------------------------------------------------

void TimerManager::updateTimers(DWORD milliseconds)
{
    // check if we need to update each timer
    //      start level and end level timers are given higher priority than the dead ghost timer
    if(mStartGameTimer > 0)
    {
        updateStartGameTimer(milliseconds);
    }
    
    else if(mEndGameTimer > 0)
    {
        updateEndGameTimer(milliseconds);
    }
    
    else if(mDeadGhostTimer > 0)
    {
        updateDeadGhostTimer(milliseconds);
    }
}

//-----------------------------------------------------------------------------------------

void TimerManager::shutdown()
{
}

//-----------------------------------------------------------------------------------------

void TimerManager::updateStartGameTimer(DWORD milliseconds)
{
    mStartGameTimer -= milliseconds;

    if(mStartGameTimer <= 0)
    {
        // timer finished, unpause game 
        const EventManager* emInstance = EventManager::ConstInstace();
        emInstance->postEvent(EventManager::GameEvent::UI_ENTER_GAME);
        emInstance->postEvent(EventManager::GameEvent::UNPAUSE);
        mStartGameTimer = 0;
    }
    else if(mStartGameTimer < START_GAME_HALF && !mHalfPauseTimePassed)
    {
        // start game pause half over, make characters visible
        const EventManager* emInstance = EventManager::ConstInstace();
        emInstance->postEvent(EventManager::GameEvent::UI_ENTER_PREGAME_2);
        emInstance->postEvent(EventManager::GameEvent::CHARACTERS_VISIBLE);
        mHalfPauseTimePassed = true;

        // if flagged to consume a life on startup, do so
        if(mWillConsumeLife)
        {
            emInstance->postEvent(EventManager::GameEvent::CONSUME_LIFE);
        }
    }
}

//-----------------------------------------------------------------------------------------

void TimerManager::updateEndGameTimer(DWORD milliseconds)
{
    mEndGameTimer -= milliseconds;

    if(mEndGameTimer <= 0)
    {
        mEndGameTimer = 0;
        if(mDidLose)
        {
            // check lives remaining if we died -- restart level or game over
            if(StatsManager::ConstInstace()->getLives() == 0)
            {
                EventManager::ConstInstace()->postEvent(EventManager::GameEvent::GAME_OVER);
            }
            else
            {
                EventManager::ConstInstace()->postEvent(EventManager::GameEvent::START_FROM_GAME_LOSE);
            }
        }
        else
        {
            // end game pause finished, reset for next level on win
            EventManager::ConstInstace()->postEvent(EventManager::GameEvent::START_FROM_GAME_WIN);
        }
    }
    else if(mEndGameTimer < END_GAME_HALF && !mHalfPauseTimePassed)
    {
        // end game pause half over, make ghosts invisible
        const EventManager* emInstance = EventManager::ConstInstace();
        emInstance->postEvent(EventManager::GameEvent::GHOSTS_INVISIBLE);
        mHalfPauseTimePassed = true;

        if(mDidLose)
        {
            // if flagged as loss, start pacman's death animation
            emInstance->postEvent(EventManager::GameEvent::PLAY_PACMAN_DEATH_ANIM);
            emInstance->postSoundEvent(EventManager::SoundEvent::SOUND_PACMAN_DEATH);
        }
        else
        {
            // otherwise, check the bonus points earned this level
            emInstance->postEvent(EventManager::GameEvent::POINTS_KILL_BONUS);
            emInstance->postEvent(EventManager::GameEvent::PLAY_TILE_FLASH);
        }
    }
}

//-----------------------------------------------------------------------------------------

void TimerManager::updateDeadGhostTimer(DWORD milliseconds)
{
    mDeadGhostTimer -= milliseconds;

    if(mDeadGhostTimer <= 0)
    {
        // short pause on ghost death finished, unpause
        EventManager::ConstInstace()->postEvent(EventManager::GameEvent::UNPAUSE);
        mDeadGhostTimer = 0;
    }
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateStartTimerFromMenu()
{
    // starting level from the menu takes the full time and consumes a life on startup
    mStartGameTimer = START_GAME_PAUSE_DURATION;
    mHalfPauseTimePassed = false;
    mWillConsumeLife = true;
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateStartTimerFromWin()
{
    // starting level from a win takes less time than usual and does not consume a life on startup
    mStartGameTimer = START_GAME_HALF - 1;
    mHalfPauseTimePassed = false;
    mWillConsumeLife = false;
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateStartTimerFromLoss()
{
    // starting level from a lost life takes less time and consumes a life on startup
    mStartGameTimer = START_GAME_HALF - 1;
    mHalfPauseTimePassed = false;
    mWillConsumeLife = true;
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateEndGameTimerWin()
{
    // level has ended, mark didLose false
    mEndGameTimer = END_GAME_PAUSE_DURATION;
    mHalfPauseTimePassed = false;
    mDidLose = false;
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateEndGameTimerLoss()
{
    // level has ended, mark didLose true
    mEndGameTimer = END_GAME_PAUSE_DURATION;
    mHalfPauseTimePassed = false;
    mDidLose = true;
}

//-----------------------------------------------------------------------------------------

void TimerManager::activateDeadGhostTimer()
{
    mDeadGhostTimer = GHOST_KILL_PAUSE_DURATION;
}
    