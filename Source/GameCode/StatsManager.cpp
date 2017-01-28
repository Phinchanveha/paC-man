
#define STATS_MANAGER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "glut.h"
#include "..\\Framework\\baseTypes.h"
#include "..\\Framework\\openglframework.h"

#include "Utilities.h"
#include "StatsManager.h"

#include <queue>
#include "EventManager.h"

#define HIGH_SCORE_FILE "Assets\\HighScore.dat"

#define POINTS_PELLET       10      // points amounts
#define POINTS_POWER_PELLET 50
#define POINTS_GHOST_KILL_1 200
#define POINTS_GHOST_KILL_2 400
#define POINTS_GHOST_KILL_3 800
#define POINTS_GHOST_KILL_4 1600
#define GHOST_KILLS_BONUS   12000

#define EXTRA_LIFE_ONE      10000   // extra life thresholds
#define EXTRA_LIFE_TWO      100000

#define MAX_GHOST_KILLS     16      // number of ghost kills for bonus


StatsManager* StatsManager::_instance = nullptr;

StatsManager::StatsManager()
{
}

//-----------------------------------------------------------------------------------------

StatsManager* StatsManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new StatsManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const StatsManager* StatsManager::ConstInstace()
{
    if(_instance == nullptr)
    {
        _instance = new StatsManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void StatsManager::init()
{
    mLevel = 0;
    mLives = 0;
    mScore = 0;
    mHighScore = 0;
    mPrevScore = 0;

    mGhostsKilledOnPellet = 0;  // counts the number of ghosts killed for the current power pellet
    mGhostKillCount = 0;        // counts total number of ghosts killed on this level

    // read from high score file
    readHighScoreFile();
}

//-----------------------------------------------------------------------------------------

void StatsManager::reset()
{
    mLevel = 1;                 // start on level 1
    mLives = 3;                 // with 3 lives (1st one is consumed on level start)
    mScore = 0;                 // with score of 0
    mPrevScore = 0;

    mTableIndex = 0;            // used to look up values in lookup tables

    mGhostsKilledOnPellet = 0;
    mGhostKillCount = 0;

    // reset all stats for level 1
    mPacmanSpeed = 0.8f;                // pacman speeds
    mPacmanSpeedDots = 0.71f;
    mPacmanSpeedScared = 0.9f;
    mPacmanSpeedScaredDots = 0.79f;

    mGhostSpeed = 0.75f;                // ghost speeds
    mGhostSpeedScared = 0.5f;
    mGhostSpeedTunnel = 0.4f;

    mCruiseElroySpeed = 0.8f;           // cruise elroy variables
    mCruiseElroyDots = 0.05f;
}

//-----------------------------------------------------------------------------------------

void StatsManager::shutdown()
{
    writeHighScoreFile();
}

//-----------------------------------------------------------------------------------------

void StatsManager::readHighScoreFile()
{
    // attempt open, if null we need to write the high score file first (writes 0)
    FILE* highScoreFile = fopen(HIGH_SCORE_FILE, "rb");
    if(highScoreFile == nullptr)
    {
        writeHighScoreFile();
    }

    // now that we're guaranteed a high score file exists, we read from it
    highScoreFile = fopen(HIGH_SCORE_FILE, "rb");
    fread(&mHighScore, sizeof(int32_t), 1, highScoreFile);
    fclose(highScoreFile);
}

//-----------------------------------------------------------------------------------------

void StatsManager::writeHighScoreFile()
{
    // write high score file
    FILE* highScoreFile = fopen(HIGH_SCORE_FILE, "wb");
    fwrite(&mHighScore, sizeof(int32_t), 1, highScoreFile);
    fclose(highScoreFile);
}

//-----------------------------------------------------------------------------------------

void StatsManager::nextLevel()
{
    // increments level number and resets the ghost kill counters
    mGhostsKilledOnPellet = 0;
    mGhostKillCount = 0;

    mLevel++;
    mTableIndex = mLevel < 21 ? mLevel - 1 : 20;

    // some stats follow a mathematical pattern and may be updated this way instead of stored in a large table
    switch(mLevel)
    {
        case 2: case 5:                         // pacman and the ghosts get faster, and cruise elroy activates earlier
            mPacmanSpeed += 0.1f;
            mPacmanSpeedDots += 0.08f;
            mPacmanSpeedScared += 0.05f;
            mPacmanSpeedScaredDots += 0.04f;

            mGhostSpeed += 0.1f;
            mGhostSpeedScared += 0.05f;
            mGhostSpeedTunnel += 0.05f;

            mCruiseElroySpeed += 0.1f;
            mCruiseElroyDots += 0.04f;
            break;

        case 3: case 6: case 9:                 // cruise elroy activates earlier
            mCruiseElroyDots += 0.04f;
            break;

        case 12: case 15: case 19:              // cruise elroy activates earlier by a greater margin
            mCruiseElroyDots += 0.08f;
            break;

        case 21:                                // level 21 reduces pacman's speed but keeps everything else the same
            mPacmanSpeed -= 0.1f;
            mPacmanSpeedDots -= 0.08f;
            mPacmanSpeedScared -= 0.05f;
            mPacmanSpeedScaredDots -= 0.04f;
            break;

        case 256:
            g_keys->keyDown[VK_ESCAPE] = true;  // kill screen at level 256
            keyProcess();
            break;
    }
}

//-----------------------------------------------------------------------------------------

void StatsManager::consumeLife()
{
    if(mLives > 0)
    {
        mLives--;
    }
}

//-----------------------------------------------------------------------------------------

void StatsManager::pelletCollected()
{
    updateScore(POINTS_PELLET);
}

//-----------------------------------------------------------------------------------------

void StatsManager::powerPelletCollected()
{
    // collecting a power pellet resets the counter for the number
    // of ghosts killed on the current power pellet
    mGhostsKilledOnPellet = 0;
    updateScore(POINTS_POWER_PELLET);
}

//-----------------------------------------------------------------------------------------

void StatsManager::ghostKilled()
{
    mGhostsKilledOnPellet++;
    mGhostKillCount++;

    switch(mGhostsKilledOnPellet)
    {
        case 1:
            updateScore(POINTS_GHOST_KILL_1);
            break;
        case 2:
            updateScore(POINTS_GHOST_KILL_2);
            break;
        case 3:
            updateScore(POINTS_GHOST_KILL_3);
            break;
        case 4:
            updateScore(POINTS_GHOST_KILL_4);
            break;
    }
}

//-----------------------------------------------------------------------------------------

void StatsManager::ghostKillBonusPoints()
{
    // bonus points if all 4 ghosts were captured at all 4 power pellets
    if(mGhostKillCount == MAX_GHOST_KILLS)
    {
        updateScore(GHOST_KILLS_BONUS);
    }
}

//-----------------------------------------------------------------------------------------

void StatsManager::fruitCollected()
{
    // different kinds of fruit are worth different points
    switch(getFruitType())
    {
        case GameUtil::FruitType::FRUIT_CHERRY:
            updateScore(100);
            break;
        case GameUtil::FruitType::FRUIT_STRAWBERRY:
            updateScore(300);
            break;
        case GameUtil::FruitType::FRUIT_PEACH:
            updateScore(500);
            break;
        case GameUtil::FruitType::FRUIT_APPLE:
            updateScore(700);
            break;
        case GameUtil::FruitType::FRUIT_GRAPES:
            updateScore(1000);
            break;
        case GameUtil::FruitType::FRUIT_GALAXIAN:
            updateScore(2000);
            break;
        case GameUtil::FruitType::FRUIT_BELL:
            updateScore(3000);
            break;
        default:
            updateScore(5000);
            break;
    }
}

//-----------------------------------------------------------------------------------------

void StatsManager::updateScore(const int32_t points)
{
    mPrevScore = mScore;
    mScore += points;

    // extra lives at 10k and 100k points
    if((mScore >= EXTRA_LIFE_ONE && mPrevScore < EXTRA_LIFE_ONE) || (mScore >= EXTRA_LIFE_TWO && mPrevScore < EXTRA_LIFE_TWO))
    {
        EventManager::ConstInstace()->postSoundEvent(EventManager::SoundEvent::SOUND_EXTRA_LIFE);
        mLives++;
    }

    // update high score
    if(mHighScore < mScore)
    {
        mHighScore = mScore;
    }
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getHighScore() const
{
    return mHighScore;
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getScore() const
{
    return mScore;
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getLives() const
{
    return mLives;
}

//-----------------------------------------------------------------------------------------

const GameUtil::FruitType StatsManager::getFruitType() const
{
    return FRUIT_TYPE_TABLE[mTableIndex];
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getGhostScaredTime() const
{
    return GHOST_SCARED_TIMES[mTableIndex];
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getGhostFlashTime() const
{
    return GHOST_SCARED_TIMES[mTableIndex] / 4;
}

//-----------------------------------------------------------------------------------------

const int32_t StatsManager::getGhostPhaseTime(int32_t phase) const
{
    assert(phase < 8);

    int32_t value = 0;
    switch(mLevel)
    {
        case 1:
            value = GHOST_PHASE_TIMES_1[phase];
            break;
        case 2: case 3: case 4:
            value = GHOST_PHASE_TIMES_2[phase];
            break;
        default:
            value = GHOST_PHASE_TIMES_3[phase];
            break;
    }


    return value;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getPacmanSpeed() const
{
    return mPacmanSpeed;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getPacmanSpeedDots() const
{
    return mPacmanSpeedDots;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getPacmanScaredSpeed() const
{
    return mPacmanSpeedScared;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getPacmanScaredSpeedDots() const
{
    return mPacmanSpeedScaredDots;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getGhostSpeed() const
{
    return mGhostSpeed;
}

//-----------------------------------------------------------------------------------------
const float_t StatsManager::getGhostSpeedScared() const
{
    return mGhostSpeedScared;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getGhostSpeedTunnel() const
{
    return mGhostSpeedTunnel;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getCruiseElroySpeed_1() const
{
    return mCruiseElroySpeed;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getCruiseElroySpeed_2() const
{
    return mCruiseElroySpeed + 0.05f;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getCruiseElroyDots_1() const
{
    return mCruiseElroyDots;
}

//-----------------------------------------------------------------------------------------

const float_t StatsManager::getCruiseElroyDots_2() const
{
    return mCruiseElroyDots / 2.0f;
}
