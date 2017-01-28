
#define NUM_TILE_ANIM_CHUNKS    2
#define NUM_TILE_TEXTURE_ROWS   6
#define NUM_TILE_TEXTURE_COLS   4

/*
 * A tile on the board of play.
 *
 * Each tile has four neighbors, which are mapped to the
 * directions UP, LEFT, DOWN, RIGHT. This way, we can call
 * on a tile's neighbors by name.
 */
class Tile
{
public:

    /*
     * Enumeration of traversal types.
     */
    typedef enum
    {
        NON_TRAVERSABLE,        // characters cannot go here
        TRAVERSABLE,            // characters can go here
        GHOST_TRAVERSABLE,      // only ghosts can go here

        INTERSECTION,           // intersection, ghost decision zone
        NO_UP_INTERSECTION,     // intersection, ghost decision zone, ghosts cannot choose up direction
        GHOST_HOME_EXIT,        // intersection, ghost decision zone, exit for ghost home
        GHOST_HOME_ENTER,       // intersection, ghost decision zone, entrance for ghost home

        TELEPORT,               // one of two warp tiles
        TUNNEL                  // tiles leading up to teleport
    }TileTraverseType;

    /*
     * Enumeration of pellet states.
     */
    typedef enum
    {
        NO_PELLET,              // no pellet
        PELLET,                 // spawns with normal pellet
        POWER_PELLET            // spawns with power pellet
    }TilePelletType;

    Tile(const Coord2D& textureOffset, const Coord2D& index, const Coord2D& boardPosition, const TileTraverseType traverse, const TilePelletType pellet);
    void init();

    /*
     * Resets the 'collected' boolean if the tile has a pellet.
     */
    void reset();

    /*
     * Updates the animation frame timer if the animation is active.
     */
    void updateTile(DWORD milliseconds);

    /*
     * Draws the tile sprite. Takes into account animation frame.
     */
    void drawTile();

    /*
     * Draws the pellet sprite if there is an uncollected pellet.
     */
    void drawPellet();

    /*
     * Destroys the neighbors map (does not destroy the neighbors,
     * just the map).
     */
    void shutdown();

    /*
     * If the tile has an uncollected pellet, checks pacman's current tile.
     * Marks the pellet as collected and returns true if pacman is present.
     * Otherwise, returns false.
     */
    bool8_t collectPellet();

    /*
     * Activates the tile's flash animation.
     */
    void activateFlash();

    /*
     * Adds the arguments to the neighbors map.
     */
    void setNeighbors(const Tile* up, const Tile* left, const Tile* down, const Tile* right);

    /*
     * Returns a pointer to the neighbor in the given direction.
     */
    const Tile* getNeighbor(GameUtil::BoardDirection neighborID) const;

    /*
     * Returns the (x,y) position of this tile in the OpenGL space.
     */
    const Coord2D& getPosition() const;

    /*
     * Returns the (x,y) board index of this tile.
     */
    const Coord2D& getBoardIndex() const;

    /*
     * Various getter functions for the tragersal type of
     * this tile.
     */
    const Tile::TileTraverseType getTraverseType() const;
    const bool isTeleporter() const;
    const bool isTunnel() const;
    const bool isTraversable() const;
    const bool isGhostTraversable() const;

    /*
     * Returns true if the tile has an uncollected pellet.
     */
    const bool isPelletPresent() const;

    /*
     * Returns the square-distance to the given Tile 'other'.
     */
    const float getSquareDistToTile(const Tile* other) const;

private:

    typedef std::map<GameUtil::BoardDirection, const Tile*> Neighbors;
    typedef std::pair<GameUtil::BoardDirection, const Tile*> NeighborsPair;
    Neighbors* mNeighbors;

    Coord2D mTextureOffset;     // percent offset from texture file origin to this tile's image
    Coord2D mPosition;
    Coord2D mBoardIndex;

    const TileTraverseType mTraverseType;
    const TilePelletType mPelletType;

    float_t mPelletSize;

    int32_t mAnimFrame;
    int32_t mTimeOnCurrFrame;

    bool8_t mIsPelletCollected;
    bool8_t mIsFlashing;
};
