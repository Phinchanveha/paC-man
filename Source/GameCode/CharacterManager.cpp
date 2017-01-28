
#define CHARACTER_MANAGER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include "SOIL.h"

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

#include <queue>
#include "EventManager.h"

#include "StatsManager.h"

#define CHARACTERS_SPRITESHEET "Assets\\Images\\CharactersSpritesheet.png"

CharacterManager* CharacterManager::_instance = nullptr;

CharacterManager::CharacterManager() :
    mCharactersSpritesheetID(0),
    mGhosts(new Ghosts())
{
}

//-----------------------------------------------------------------------------------------

CharacterManager* CharacterManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new CharacterManager();
    }
    
    return _instance;
}

//-----------------------------------------------------------------------------------------

const CharacterManager* CharacterManager::ConstInstance()
{
    // returning a const-safe instance of the CharacterManagr
    //      this way, the caller can't do anything it shouldn't to the manager
    if(_instance == nullptr)
    {
        _instance = new CharacterManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void CharacterManager::init()
{
    mCharactersSpritesheetID = SOIL_load_OGL_texture(CHARACTERS_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mCharactersSpritesheetID > 0);

    // create pacman object
    mPacman = new Pacman(BoardManager::ConstInstace()->getCharacterSpawnTile(GameUtil::CharacterID::PACMAN), GameUtil::CharacterID::PACMAN);

    // create ghost objects
    mGhosts->insert(IDGhostPair(GameUtil::CharacterID::GHOST_RED,                                               // key for character manager map
        new GhostRed(BoardManager::ConstInstace()->getCharacterSpawnTile(GameUtil::CharacterID::GHOST_RED),     // getting spawn point for this ghost
            GameUtil::CharacterID::GHOST_RED)));                                                                // giving ghost its character ID

    mGhosts->insert(IDGhostPair(GameUtil::CharacterID::GHOST_PINK,
        new GhostPink(BoardManager::ConstInstace()->getCharacterSpawnTile(GameUtil::CharacterID::GHOST_PINK),
            GameUtil::CharacterID::GHOST_PINK)));

    mGhosts->insert(IDGhostPair(GameUtil::CharacterID::GHOST_BLUE,
        new GhostBlue(BoardManager::ConstInstace()->getCharacterSpawnTile(GameUtil::CharacterID::GHOST_BLUE),
            GameUtil::CharacterID::GHOST_BLUE)));

    mGhosts->insert(IDGhostPair(GameUtil::CharacterID::GHOST_ORANGE,
        new GhostOrange(BoardManager::ConstInstace()->getCharacterSpawnTile(GameUtil::CharacterID::GHOST_ORANGE),
            GameUtil::CharacterID::GHOST_ORANGE)));
}

//-----------------------------------------------------------------------------------------

void CharacterManager::reset()
{
    mIsPaused = true;

    // reset ghost timer variables
    mGhostTimerPhase = GhostTimerPhase::SCATTER_1;

    const StatsManager* smInstance = StatsManager::ConstInstace();
    mPhaseTimer = smInstance->getGhostPhaseTime(GhostTimerPhase::SCATTER_1);
    mScatterTheshold_2 = smInstance->getGhostPhaseTime(GhostTimerPhase::SCATTER_2);
    mScatterTheshold_3 = smInstance->getGhostPhaseTime(GhostTimerPhase::SCATTER_3);
    mScatterTheshold_4 = smInstance->getGhostPhaseTime(GhostTimerPhase::SCATTER_4);
    mChaseThreshold_1 = smInstance->getGhostPhaseTime(GhostTimerPhase::CHASE_1);
    mChaseThreshold_2 = smInstance->getGhostPhaseTime(GhostTimerPhase::CHASE_2);
    mChaseThreshold_3 = smInstance->getGhostPhaseTime(GhostTimerPhase::CHASE_3);

    // reset each character for the next level
    mPacman->reset();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->reset();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::resetFromDeath()
{
    mIsPaused = true;

    // reset each character to restart the same level
    mPacman->resetFromDeath();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->resetFromDeath();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacters(DWORD milliseconds)
{
    updateCharacterTimers(milliseconds);        // updates timers (mostly for ghosts)
    updateCharacterMovement(milliseconds);      // updates movement related things -- position and tile pointers
    updateCharacterAnimation(milliseconds);     // updates character animations
    updateCharacterSpeed();                     // updates character speed values
    updateCharacterCollision();                 // detects collisions between pacman and ghosts
}

//-----------------------------------------------------------------------------------------

void CharacterManager::drawCharacters()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mCharactersSpritesheetID);
    glBegin(GL_QUADS);

    mPacman->drawCharacter();

    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->drawCharacter();
    }

    glEnd();
}

//-----------------------------------------------------------------------------------------

void CharacterManager::shutdown()
{
    mPacman->shutdown();
    delete mPacman;

    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->shutdown();
        delete iter->second;
    }

    delete mGhosts;
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacterTimers(DWORD milliseconds)
{
    mPacman->updateTimers(milliseconds);
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->updateTimers(milliseconds);
    }

    // if not paused and ghosts not scared, update the ghost phase timer
    if(!mIsPaused && !areGhostsScared())
    {
        if(mPhaseTimer > 0)
        {
            mPhaseTimer -= milliseconds;
            switch(mGhostTimerPhase)
            {
                // each time we cross a timer threshold, we move to the next targeting phase
                case GhostTimerPhase::SCATTER_1:
                    if(mPhaseTimer < mChaseThreshold_1) toggleGhostsTargetingPhase();
                    break;
                case GhostTimerPhase::CHASE_1:
                    if(mPhaseTimer < mScatterTheshold_2) toggleGhostsTargetingPhase();
                    break;
                case GhostTimerPhase::SCATTER_2:
                    if(mPhaseTimer < mChaseThreshold_2) toggleGhostsTargetingPhase();
                    break;
                case GhostTimerPhase::CHASE_2:
                    if(mPhaseTimer < mScatterTheshold_3) toggleGhostsTargetingPhase();
                    break;
                case GhostTimerPhase::SCATTER_3:
                    if(mPhaseTimer < mChaseThreshold_3) toggleGhostsTargetingPhase();
                    break;
                case GhostTimerPhase::CHASE_3:
                    if(mPhaseTimer < mScatterTheshold_4) toggleGhostsTargetingPhase();
                    break;
                default:
                    break;
            }
        }
        else
        {
            // enters final state (CHASE_4) indefinitely
            if(mGhostTimerPhase == GhostTimerPhase::SCATTER_4)
            {
                toggleGhostsTargetingPhase();
                mPhaseTimer = 0;
            }
        }
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacterMovement(DWORD milliseconds)
{
    mPacman->updateMovement(milliseconds);
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->updateMovement(milliseconds);
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacterAnimation(DWORD milliseconds)
{
    mPacman->updateAnimation(milliseconds);
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->updateAnimation(milliseconds);
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacterSpeed()
{
    mPacman->updateSpeed();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->updateSpeed();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::updateCharacterCollision()
{
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        // collision occurs if pacman's current tile and ghost's current tile are the same
        if(mPacman->getCurrentTile() == iter->second->getCurrentTile())
        {
            switch(iter->second->getTargetingState())
            {
                case Ghost::GhostTargetingState::SCARED:
                    killGhost(iter->second);
                    break;
                case Ghost::GhostTargetingState::DEAD:
                    break;
                default:
                    if(isCharacterAlive(GameUtil::CharacterID::PACMAN))
                    {
                        EventManager::ConstInstace()->postEvent(EventManager::GameEvent::LEVEL_LOSE);
                        mPacman->setLiving(false);
                    }
                    break;
            }
        }
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::pauseCharacters()
{
    mIsPaused = true;

    mPacman->pause();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->pause();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::unpauseCharacters()
{
    mIsPaused = false;

    mPacman->unpause();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->unpause();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::makeCharactersVisible() const
{
    mPacman->makeVisible();
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->makeVisible();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::makeGhostsInvisible() const
{
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->makeInvisible();
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::toggleGhostsTargetingPhase()
{
    // enter the next targeting phase
    mGhostTimerPhase = (GhostTimerPhase)(mGhostTimerPhase + 1);
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        // flag for reverse, then set scatter or chase based on the current phase
        iter->second->flagForDirectionReverse();

        switch(mGhostTimerPhase)
        {
            case GhostTimerPhase::SCATTER_1: case GhostTimerPhase::SCATTER_2:
            case GhostTimerPhase::SCATTER_3: case GhostTimerPhase::SCATTER_4:
                iter->second->setScatter();
                break;

            case GhostTimerPhase::CHASE_1: case GhostTimerPhase::CHASE_2:
            case GhostTimerPhase::CHASE_3: case GhostTimerPhase::CHASE_4:
                iter->second->setChasing();
                break;
        }
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::playPacmanDeathAnimation() const
{
    dynamic_cast<Pacman*>(mPacman)->playDeathAnimation();
}

//-----------------------------------------------------------------------------------------

void CharacterManager::spawnGhost(GameUtil::CharacterID ghostID) const
{
    if(ghostID != GameUtil::CharacterID::PACMAN)
    {
        mGhosts->at(ghostID)->setLiving(true);
    }
}

//-----------------------------------------------------------------------------------------

void CharacterManager::killGhost(Ghost* ghost)
{
    // post event that a ghost has been killed
    const EventManager* emManager = EventManager::ConstInstace();
    emManager->postEvent(EventManager::GameEvent::GHOST_KILLED);
    emManager->postSoundEvent(EventManager::SoundEvent::SOUND_GHOST_DEATH);

    // pause characters, then kill and unpause given ghost
    pauseCharacters();

    ghost->setLiving(false);
    ghost->unpause();
}

//-----------------------------------------------------------------------------------------

void CharacterManager::scareGhosts() const
{
    // scare ghosts (only sets value if ghost is outside ghost home)
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        iter->second->scare();
    }
}

//-----------------------------------------------------------------------------------------

const bool8_t CharacterManager::areGhostsScared() const
{
    bool8_t result = false;
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        if(iter->second->isGhostScared())
        {
            result = true;
            break;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------------------

void CharacterManager::activateCruiseElroy() const
{
    // get the red ghost out of the ghosts map, then cast it to red so we can set his elroy mode
    //      this function bumps up the cruise elroy phase to the next highest level if it is able
    dynamic_cast<GhostRed*>(mGhosts->at(GameUtil::CharacterID::GHOST_RED))->activateCruiseElroy();
}

//-----------------------------------------------------------------------------------------

const GameUtil::CruiseElroyMode CharacterManager::getCruiseElroyPhase() const
{
    // get the red ghost out of the ghosts map, then cast it to red so we can get his elroy mode
    return dynamic_cast<GhostRed*>(mGhosts->at(GameUtil::CharacterID::GHOST_RED))->getCruiseElroyPhase();
}

//-----------------------------------------------------------------------------------------

const bool8_t CharacterManager::areAllGhostsSpawned() const
{
    // this is important for cruise elroy
    //      the red ghost speeds up only if the other ghosts are outside the ghost house
    bool8_t result = true;
    for(auto iter = mGhosts->begin(); iter != mGhosts->end(); ++iter)
    {
        // if any ghost is in the ghost home, return false
        if(iter->second->isInGhostHome())
        {
            result = false;
            break;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------------------

const bool8_t CharacterManager::isCharacterAlive(GameUtil::CharacterID character) const
{
    bool8_t result = false;
    if(character == GameUtil::CharacterID::PACMAN)
    {
        result = mPacman->isAlive();
    }
    else
    {
        result = mGhosts->at(character)->isAlive();
    }

    return result;
}

//-----------------------------------------------------------------------------------------

const Tile* CharacterManager::getCharacterCurrentTile(GameUtil::CharacterID character) const
{
    const Tile* tile = nullptr;
    if(character == GameUtil::CharacterID::PACMAN)
    {
        tile = mPacman->getCurrentTile();
    }
    else
    {
        tile = mGhosts->at(character)->getCurrentTile();
    }

    return tile;
}

//-----------------------------------------------------------------------------------------

const GameUtil::BoardDirection CharacterManager::getCharacterFacingDirection(GameUtil::CharacterID character) const
{
    GameUtil::BoardDirection direction = GameUtil::BoardDirection::INVAL;
    if(character == GameUtil::CharacterID::PACMAN)
    {
        direction = mPacman->getFacingDirection();
    }
    else
    {
        direction = mGhosts->at(character)->getFacingDirection();
    }

    return direction;
}

//-----------------------------------------------------------------------------------------

const Character::MovementState CharacterManager::getCharacterMovementState(GameUtil::CharacterID character) const
{
    Character::MovementState movementState = Character::MOVING;
    if(character == GameUtil::CharacterID::PACMAN)
    {
        movementState = mPacman->getMovementState();
    }
    else
    {
        movementState = mGhosts->at(character)->getMovementState();
    }

    return movementState;
}
