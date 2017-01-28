
#define NUM_BOARD_ROWS 36
#define NUM_BOARD_COLS 28

/*
 * Stores a graph of tiles for the field of play.
 * Handles the updates for tiles, pellets, and 
 * board-related events.
 */
class BoardManager
{
public:

    static BoardManager* Instance();
    static const BoardManager* ConstInstace();

    /*
     * Reads files to map different types of tiles 
     * with naviation and pellet data.
     */
    void init();

    /*
     * Resets the board for the next level by turning
     * on pellets.
     */
    void reset();

    /*
     * Calls update on each tile in the board.
     */
    void updateBoard(DWORD milliseconds);

    /*
     * Draws tiles, then draws pellets on top.
     */
    void drawBoard();

    /*
     * Clears maps and destroys tiles.
     */
    void shutdown();

    /*
     * Tells all tiles to begin their flashing animation
     * when a level is cleared.
     */
    void flashTiles() const;

    /*
     * Character spawn tiles are mapped to a character ID.
     * Returns the spawn tile for the given character ID.
     */
    const Tile* getCharacterSpawnTile(GameUtil::CharacterID characterID) const;

    /*
     * Ghosts' home tiles (the tile they target in SCATTER mode) are mapped
     * to character IDs. Returns the home tile for the given character ID.
     *
     * The PACMAN ID is the the home tile inside the ghost home, which is
     * what ghosts target while DEAD, so they move to the ghost home.
     */
    const Tile* getGhostHomeTile(GameUtil::CharacterID characterID) const;

    /*
     * Returns the tile marked as the fruit spawn tile.
     */
    const Tile* getFruitTile() const;
    
    /*
     * Returns the percentage of pellets that remain on the board. Used
     * to trigger different events.
     */
    const float_t getPercentPelletsRemaining() const;

    /*
     * Iteratively looks from 'tile' at neighbors in 'direction' 'num' times.
     * Stops when the number of tiles ahead has been met, or nullptr is encountered.
     *
     * (For example, finding the tile 4 spaces LEFT of the given 'tile'.)
     */
    const Tile* lookNumTilesAhead(const Tile* tile, const uint32_t num, const GameUtil::BoardDirection direction) const;

private:

    BoardManager();

    /*
     * Calls on pacman's current tile to check if its pellet needs to be collected.
     */
    void checkPelletCollected();

    /*
     * Triggers events such as ghost spawns, fruit spawns, and Cruise Elroy
     * based on the number of pellets remaining on the board.
     */
    void checkPelletTriggerEvents();

    /*
     * Init method that creates the tile objects and assigns them
     * sprite and pellet data. Places each tile in the mBoard array.
     */
    void createTiles();

    /*
     * Init method that does a second pass through mBoard and connects
     * each tile to its neighbors as a graph.
     */
    void connectTiles();

    /*
     * Loads the tile texture, and reads the TileMappings file to map
     * tile codes with texture coordinates.
     */
    void constructCharTextureMap();

    /*
     * Calculates the true [0,1] texture coordinates of a tile
     * based on its mapped texture coordinates.
     */
    Coord2D getTileTextureOffset(float x, float y);         // x and y here are indices in the tile spritesheet

    static BoardManager* _instance;

    Coord2D mBoardPosition;

    Tile* mBoard[NUM_BOARD_ROWS][NUM_BOARD_COLS];

    typedef std::map<GameUtil::CharacterID, Tile*> GhostHomes;           // maps ghost ID number to a particular tile (ghost's "home" for scatter behavior)
    typedef std::pair<GameUtil::CharacterID, Tile*> GhostHomesPair;
    GhostHomes* mGhostHomes;

    typedef std::map<GameUtil::CharacterID, Tile*> CharacterSpawns;      // maps character ID numbers to spawn points
    typedef std::pair<GameUtil::CharacterID, Tile*> CharacterSpawnsPair;
    CharacterSpawns* mCharacterSpawns;

    typedef std::map<char8_t, Coord2D> TextureIndexMap;                 // maps alphanumerical characters to texture indicies in a loaded bitmap
    typedef std::pair<char8_t, Coord2D> CharIndexPair;
    TextureIndexMap* mTextureIndexMap;

    const Tile* mFruitTile;                                             // location of spawned fruit

    int32_t mNumPellets;
    int32_t mMaxNumPellets;

    float_t mCruiseElroyThreshold_1;        // saved values from the stats manager -- dot amounts needed to activate cruise elroy
    float_t mCruiseElroyThreshold_2;

    GLuint mTileSpritesheetID;
    GLuint mPelletImageID;

    bool8_t mDidPostWinEvent;
};
