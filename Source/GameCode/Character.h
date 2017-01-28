
#define NUM_CHARACTERS              9       // number of character animation chunk regions
#define NUM_ANIM_STATES             4       // number of animations per animation chunk region

#define NUM_ANIM_FRAMES             6       // number of frames per animation state (number of rows)

#define CHARACTER_CHUNK_SIZE        1.0f/NUM_CHARACTERS                         // % width of a character chunk compared to the total spritesheet
#define CHARACTER_COL_SIZE          1.0f/(NUM_CHARACTERS*NUM_ANIM_STATES)       // % width of a character sprite compared to the total spritesheet
#define CHARACTER_ROW_SIZE          1.0f/NUM_ANIM_FRAMES                        // % height of a character sprite compared to the total spritesheet

#define CHARACTER_SPRITE_SIZE               48
#define CHARACTER_SPRITE_OFFSET_FACTOR      CHARACTER_SPRITE_SIZE*3.25f
#define HALF_CHARACTER_SPRITE_SIZE          CHARACTER_SPRITE_OFFSET_FACTOR/2.0f

/*
 * Base class for all characters that can
 * move around the board.
 */
class Character
{
public:

    /*
     * Enumeration for the character's movement state.
     * When they are MOVING, the character physically moves.
     * When they are AT_TILE, the character may choose their
     * next movement target.
     */
    typedef enum
    {
        MOVING,
        AT_TILE
    }MovementState;

    /*
     * Constructor.
     */
    Character(const Tile* spawnTile, const GameUtil::CharacterID id);

    /*
     * Initializes values for the character.
     */
    void init();

    /*
     * Resets variables on the character for a new level.
     */
    virtual void reset();

    /*
     * Resets variables on the character when restarting
     * the same level, such as when pacman dies.
     */
    virtual void resetFromDeath();

    /*
     * Updates any timers that the character may be running.
     */
    virtual void updateTimers(DWORD milliseconds);

    /*
     * Updates movement-related things, such as position,
     * movement state, and tiles. Calls on helper functions
     * to handle the update based on the movement state.
     */
    virtual void updateMovement(DWORD milliseconds);

    /*
     * Updates the delta time between the character's 
     * animation frames.
     */
    virtual void updateAnimation(DWORD milliseconds);

    /*
     * Virtual function. Characters update their speed
     * here, based on a number of factors.
     */
    virtual void updateSpeed() = 0;

    /*
     * Draws the character to the screen. The draw call
     * is ignored if the character is marked as invisible.
     */
    void drawCharacter();

    /*
     * Clears any allocated members and preps object
     * for shutdown.
     */
    void shutdown();

    /*
     * Turns off the boolean controlling the character's
     * updates.
     */
    void unpause();

    /*
    * Turns on the boolean controlling the character's update.
    * Paused characters do not update their position, timers, 
    * or animation frame.
    */
    void pause();

    /*
     * Turns on the boolean that allows the character's draw
     * method to run.
     */
    void makeVisible();

    /*
     * Turns off the boolean that allows the character's draw
     * method to run.
     */
    void makeInvisible();

    /*
     * Getter and seter for the character's living status.
     */
    const bool8_t isAlive() const;
    virtual void setLiving(bool alive);

    /*
     * Getter for the character ID.
     */
    const GameUtil::CharacterID getCharacterID() const;

    /*
     * Getters for movement variables, such as movement tile,
     * facing direction, and movement state.
     */
    const Tile* getCurrentTile() const;
    const GameUtil::BoardDirection getFacingDirection() const;
    const MovementState getMovementState() const;

private:

    /*
     * Physically moves the character in the direction
     * given by mFacingDirection.
     */
    void handleMovement(DWORD milliseconds);

    /*
     * Virtual function. When in AT_TILE, characters update their
     * mNextTile movement target and then reenter the MOVING state.
     */
    virtual void chooseNextTile() = 0;

protected:

    const GameUtil::CharacterID mCharacterID;

    GameUtil::AnimChunkID mAnimChunkID;
    uint32_t mAnimFrame;
    uint32_t mTimeOnCurrFrame;
    uint32_t mFrameDeltaTime;

    GameUtil::BoardDirection mFacingDirection;
    MovementState mMovementState;

    Coord2D mPosition;

    float mBaseSpeed;
    float mSpeed;

    const Tile* mSpawnTile;
    const Tile* mPrevTile;      // tile that the character is moving from
    const Tile* mCurrTile;      // tile that the character is currently at
    const Tile* mNextTile;      // tile that the character is moving towards

    bool8_t mIsPaused;
    bool8_t mIsVisible;
    bool8_t mIsAlive;

    bool8_t mDidTeleport;
};
