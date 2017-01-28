
#define GHOST_ORANGE_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "Character.h"
#include "Ghost.h"
#include "GhostOrange.h"

#include "CharacterManager.h"
#include "BoardManager.h"

#define EIGHT_SPRITES_SQR_DIST CHARACTER_SPRITE_OFFSET_FACTOR*CHARACTER_SPRITE_OFFSET_FACTOR*8

GhostOrange::GhostOrange(const Tile* spawnTile, const GameUtil::CharacterID id) :
    Ghost(spawnTile, id)
{
    // CLYDE
    // POKEY
}

//-----------------------------------------------------------------------------------------

void GhostOrange::reset()
{
    Ghost::reset();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostOrange::resetFromDeath()
{
    Ghost::resetFromDeath();

    mIsInGhostHome = true;
    mIsAlive = false;
}

//-----------------------------------------------------------------------------------------

void GhostOrange::targetPacman()
{
    // target pacman directly at first
    mTargetTile = CharacterManager::ConstInstance()->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);
    
    Coord2D targetPosition = mTargetTile->getPosition();
    float distToTarget = GameUtil::Utilities::squareDistance(targetPosition.x, mPosition.x, targetPosition.y, mPosition.y);

    // if we're close enough to the target, target home corner instead
    if(distToTarget < EIGHT_SPRITES_SQR_DIST)
    {
        mTargetTile = BoardManager::ConstInstace()->getGhostHomeTile(mCharacterID);
    }
}
