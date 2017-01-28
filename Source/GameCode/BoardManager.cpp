
#define BOARD_MANAGER_CPP

#include <stdio.h>
#include <windows.h>											// Header File For Windows
#include <gl\gl.h>											    // Header File For The OpenGL32 Library
#include <gl\glu.h>												// Header File For The GLu32 Library
#include <assert.h>
#include "..\\Framework\\baseTypes.h"

#include "soil.h"

#include <map>
#include "Utilities.h"
#include "Tile.h"
#include "BoardManager.h"

#include "FruitManager.h"

#include "Character.h"
#include "Pacman.h"
#include "Ghost.h"
#include "GhostRed.h"
#include "GhostPink.h"
#include "GhostBlue.h"
#include "GhostOrange.h"
#include "CharacterManager.h"

#include <queue>
#include "EventManager.h"

#include "StatsManager.h"

#define PACMAN_BOARD_FILE "Assets\\PacmanBoard.txt"
#define TILE_MAPPINGS_FILE "Assets\\TileMappings.txt"
#define TILES_SPRITESHEET "Assets\\Images\\TilesSpritesheet.png"
#define PELLET_IMAGE "Assets\\Images\\Pellet.png"

#define BOARD_X 0
#define BOARD_Y 0

BoardManager* BoardManager::_instance = nullptr;

BoardManager::BoardManager() :
    mTileSpritesheetID(0),
    mPelletImageID(0),
    mGhostHomes(new GhostHomes()),
    mCharacterSpawns(new CharacterSpawns()),
    mTextureIndexMap(new TextureIndexMap()),
    mNumPellets(0),
    mMaxNumPellets(0)
{
}

//-----------------------------------------------------------------------------------------

BoardManager* BoardManager::Instance()
{
    if(_instance == nullptr)
    {
        _instance = new BoardManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

const BoardManager* BoardManager::ConstInstace()
{
    // returning a const-safe instance of the BoardManager
    //      this way, the caller can't do anything it shouldn't to the manager
    if(_instance == nullptr)
    {
        _instance = new BoardManager();
    }

    return _instance;
}

//-----------------------------------------------------------------------------------------

void BoardManager::init()
{
    mBoardPosition.y = BOARD_Y;
    mBoardPosition.x = BOARD_X;

    constructCharTextureMap();
    createTiles();
    connectTiles();
}

//-----------------------------------------------------------------------------------------

void BoardManager::reset()
{
    // reset each tile
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->reset();
        }
    }

    // reset number of pellets
    mNumPellets = mMaxNumPellets;
    mDidPostWinEvent = false;

    // save the cruise elroy thresholds from the stats manager
    const StatsManager* smInstance = StatsManager::ConstInstace();
    mCruiseElroyThreshold_1 = smInstance->getCruiseElroyDots_1();
    mCruiseElroyThreshold_2 = smInstance->getCruiseElroyDots_2();
}

//-----------------------------------------------------------------------------------------

void BoardManager::updateBoard(DWORD milliseconds)
{
    checkPelletCollected();
    checkPelletTriggerEvents();

    // update tiles
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->updateTile(milliseconds);
        }
    }
}

//-----------------------------------------------------------------------------------------

void BoardManager::drawBoard()
{
    // bind the single tile spritesheet beforehand so we don't load and reload stuff for individual tiles
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mTileSpritesheetID);
    glBegin(GL_QUADS);

    // call draw on each tile object
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->drawTile();
        }
    }

    glEnd();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, mPelletImageID);
    glBegin(GL_QUADS);

    // call draw for each tile's pellet
    //      have to separate because the first pass is in a different OpenGL draw mode
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->drawPellet();
        }
    }

    glEnd();
}

//-----------------------------------------------------------------------------------------

void BoardManager::shutdown()
{
    // shutdown each tile before destroying it
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->shutdown();
            delete mBoard[y][x];
        }
    }

    // destroy maps -- any tiles stored in the maps are already destroyed
    delete mGhostHomes;
    delete mCharacterSpawns;
    delete mTextureIndexMap;
}

//-----------------------------------------------------------------------------------------

void BoardManager::checkPelletCollected()
{
    const CharacterManager* cmInstance = CharacterManager::ConstInstance();

    // need to check pacman's movement state
    //      we don't want to collect the pellet too early, because the pellet needs to affect pacman's movement speed
    const Character::MovementState movementState = cmInstance->getCharacterMovementState(GameUtil::CharacterID::PACMAN);
    if(movementState == Character::AT_TILE)
    {
        const Tile* pacmanTile = cmInstance->getCharacterCurrentTile(GameUtil::CharacterID::PACMAN);

        const Coord2D& tileIndex = pacmanTile->getBoardIndex();
        if(mBoard[(int)tileIndex.y][(int)tileIndex.x]->collectPellet())
        {
            mNumPellets--;
        }
    }
}

//-----------------------------------------------------------------------------------------

void BoardManager::checkPelletTriggerEvents()
{
    // some events are triggered by certain numbers of pellets being collected
    //      we check those here

    const EventManager* emInstance = EventManager::ConstInstace();
    const CharacterManager* cmInstance = CharacterManager::ConstInstance();

    float_t remainingPellets = getPercentPelletsRemaining();

    // ghost spawns
    if(remainingPellets < 2.0f)
    {
        if(!cmInstance->isCharacterAlive(GameUtil::CharacterID::GHOST_RED))
            emInstance->postEvent(EventManager::GameEvent::SPAWN_GHOST_RED);

        if(!cmInstance->isCharacterAlive(GameUtil::CharacterID::GHOST_PINK))
            emInstance->postEvent(EventManager::GameEvent::SPAWN_GHOST_PINK);
    }
    if(remainingPellets < 0.87f)
    {
        if(!cmInstance->isCharacterAlive(GameUtil::CharacterID::GHOST_BLUE))
            emInstance->postEvent(EventManager::GameEvent::SPAWN_GHOST_BLUE);
    }
    if(remainingPellets < 0.66f)
    {
        if(!cmInstance->isCharacterAlive(GameUtil::CharacterID::GHOST_ORANGE))
            emInstance->postEvent(EventManager::GameEvent::SPAWN_GHOST_ORANGE);
    }

    // activating cruise elroy mode for red ghost
    switch (cmInstance->getCruiseElroyPhase())
    {
        case GameUtil::CruiseElroyMode::NONE:
            if(remainingPellets < mCruiseElroyThreshold_1) emInstance->postEvent(EventManager::GameEvent::CRUISE_ELROY);
            break;
        case GameUtil::CruiseElroyMode::CRUISE_1:
            if(remainingPellets < mCruiseElroyThreshold_2) emInstance->postEvent(EventManager::GameEvent::CRUISE_ELROY);
            break;
    }

    // activating fruit
    switch(FruitManager::ConstInstace()->getPhase())
    {
        case GameUtil::FruitPhase::PRE_FRUIT:
            if(remainingPellets < 0.71f) emInstance->postEvent(EventManager::GameEvent::ACTIVATE_FRUIT);
            break;
        case GameUtil::FruitPhase::FRUIT_1:
            if(remainingPellets < 0.3f) emInstance->postEvent(EventManager::GameEvent::ACTIVATE_FRUIT);
            break;
    }

    // check win event
    if(remainingPellets == 0.0f)
    {
        if(!mDidPostWinEvent)
        {
            emInstance->postEvent(EventManager::GameEvent::LEVEL_WIN);
            mDidPostWinEvent = true;
        }
    }
}

//-----------------------------------------------------------------------------------------

void BoardManager::flashTiles() const
{
    // set all tiles to flash
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            mBoard[y][x]->activateFlash();
        }
    }
}

//-----------------------------------------------------------------------------------------

const Tile* BoardManager::getCharacterSpawnTile(GameUtil::CharacterID characterID) const
{
    return mCharacterSpawns->at(characterID);
}

//-----------------------------------------------------------------------------------------

const Tile* BoardManager::getGhostHomeTile(GameUtil::CharacterID characterID) const
{
    return mGhostHomes->at(characterID);
}

//-----------------------------------------------------------------------------------------

const Tile* BoardManager::getFruitTile() const
{
    return mFruitTile;
}

//-----------------------------------------------------------------------------------------

const float_t BoardManager::getPercentPelletsRemaining() const
{
    return (float_t)mNumPellets / (float_t)mMaxNumPellets;
}

//-----------------------------------------------------------------------------------------

const Tile* BoardManager::lookNumTilesAhead(const Tile* tile, const uint32_t num, const GameUtil::BoardDirection direction) const
{
    const Tile* returnTile = tile;
    const Tile* tempTile = nullptr;

    // step forward in the given direction until null is encountered
    //      O(n), but 'num' is small and acts as a barrier against accidental array out-of-bounds
    for(uint32_t iter = 0; iter < num; ++iter)
    {
        tempTile = returnTile->getNeighbor(direction);
        if(tempTile == nullptr)
        {
            break;
        }

        returnTile = tempTile;
    }

    return returnTile;
}




//-----------------------------------------------------------------------------------------

void BoardManager::createTiles()
{
    FILE* boardFile = fopen(PACMAN_BOARD_FILE, "r");
    char8_t buffer[NUM_BOARD_COLS + 8];

    char8_t currentTileChar;
    Coord2D currentTileIndex;

    mMaxNumPellets = 0;

    // first pass on board, init grid array of tile pointers using file
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        fscanf(boardFile, "%s", buffer);
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            currentTileChar = buffer[x];
            currentTileIndex.x = x;
            currentTileIndex.y = y;

            // set pellet type
            Tile::TilePelletType pelletType;
            switch(currentTileChar)
            {
                case 'O':
                    pelletType = Tile::TilePelletType::POWER_PELLET;
                    mMaxNumPellets++;
                    break;
                case 'S': case 'X': case 'o':
                    pelletType = Tile::TilePelletType::PELLET;
                    mMaxNumPellets++;
                    break;
                default:
                    pelletType = Tile::TilePelletType::NO_PELLET;
                    break;
            }

            // set tile traversal type
            Tile::TileTraverseType traverseType;
            switch(currentTileChar)
            {
                case 'f': case '5': case '6': case '7': case '8': 
                case '9': case '.': case 'o': case 'O':
                    traverseType = Tile::TileTraverseType::TRAVERSABLE;
                    break;
                case '/':
                    traverseType = Tile::TileTraverseType::GHOST_TRAVERSABLE;
                    break;
                case 'T':
                    traverseType = Tile::TileTraverseType::TELEPORT;
                    break;
                case 't':
                    traverseType = Tile::TileTraverseType::TUNNEL;
                    break;
                case 'x': case 'X':
                    traverseType = Tile::TileTraverseType::INTERSECTION;
                    break;
                case 's': case 'S':
                    traverseType = Tile::TileTraverseType::NO_UP_INTERSECTION;
                    break;
                case '^':
                    traverseType = Tile::TileTraverseType::GHOST_HOME_EXIT;
                    break;
                case 'v':
                    traverseType = Tile::TileTraverseType::GHOST_HOME_ENTER;
                    break;
                default:
                    traverseType = Tile::TileTraverseType::NON_TRAVERSABLE;
                    break;
            }

            // create new tile object at the current board position, save it in any relevant maps
            mBoard[y][x] = new Tile(mTextureIndexMap->at(currentTileChar), currentTileIndex, mBoardPosition, traverseType, pelletType);
            switch(currentTileChar)
            {
                case '0': case '1': case '2': case '3': case '4':
                    mGhostHomes->insert(GhostHomesPair((GameUtil::CharacterID)(currentTileChar - '0'), mBoard[y][x]));
                    break;
                case '5': case '6': case '7': case '8': case '9':
                    mCharacterSpawns->insert(CharacterSpawnsPair((GameUtil::CharacterID)(currentTileChar - '5'), mBoard[y][x]));
                    break;
                case 'f':
                    mFruitTile = mBoard[y][x];
                    break;
            }
        }
    }

    fclose(boardFile);
}

//-----------------------------------------------------------------------------------------

void BoardManager::connectTiles()
{
    Tile* up;
    Tile* left;
    Tile* down;
    Tile* right;

    // second pass on board, connect tiles to their neighbors
    for(int y = 0; y < NUM_BOARD_ROWS; ++y)
    {
        for(int x = 0; x < NUM_BOARD_COLS; ++x)
        {
            up = y > 0 ? mBoard[y - 1][x] : nullptr;
            left = x > 0 ? mBoard[y][x - 1] : nullptr;
            down = y < NUM_BOARD_ROWS - 1 ? mBoard[y + 1][x] : nullptr;
            right = x < NUM_BOARD_COLS - 1 ? mBoard[y][x + 1] : nullptr;

            // special case for left and right neighbors of teleport tiles
            if(mBoard[y][x]->isTeleporter())
            {
                if(x == 0)
                {
                    // left teleporter's left neighbor is right teleporter
                    left = mBoard[y][NUM_BOARD_COLS - 1];
                }
                else
                {
                    // right teleporter's right neighbor is left teleporter
                    right = mBoard[y][0];
                }
            }

            mBoard[y][x]->setNeighbors(up, left, down, right);
        }
    }
}

//-----------------------------------------------------------------------------------------

void BoardManager::constructCharTextureMap()
{
    mTileSpritesheetID = SOIL_load_OGL_texture(TILES_SPRITESHEET, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mTileSpritesheetID > 0);

    mPelletImageID = SOIL_load_OGL_texture(PELLET_IMAGE, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
    assert(mPelletImageID > 0);

    // read map file, associating each alphanumerical key with an (x,y) offset index into the tile spritesheet
    FILE* tileMapFile = fopen(TILE_MAPPINGS_FILE, "r");

    char8_t key;
    int xOffset, yOffset;
    while(fscanf(tileMapFile, "%c %d %d\n", &key, &xOffset, &yOffset) != EOF)
    {
        mTextureIndexMap->insert(CharIndexPair(key, getTileTextureOffset(xOffset, yOffset)));
    }

    fclose(tileMapFile);
}

//-----------------------------------------------------------------------------------------

Coord2D BoardManager::getTileTextureOffset(float x, float y)
{
    Coord2D textureCoord;

    // store the texture coord as the % offset from the tile texture file start
    //      y offset is backwards, need to compensate
    textureCoord.x = x / (NUM_TILE_TEXTURE_COLS * NUM_TILE_ANIM_CHUNKS);
    textureCoord.y = (NUM_TILE_TEXTURE_ROWS - y - 1) / NUM_TILE_TEXTURE_ROWS;

    return textureCoord;
}
