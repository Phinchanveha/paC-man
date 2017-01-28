
/*
 * Base class for all ghost characters.
 */
class Ghost : public Character
{
public:

    /*
     * Enumeration by which a ghost chooses its target 
     * tile. The target tile influences how the ghost 
     * chooses its next tile at intersections.
     */
    typedef enum
    {
        CHASE,
        SCATTER,
        SCARED,
        DEAD
    }GhostTargetingState;

    Ghost(const Tile* spawnTile, const GameUtil::CharacterID id);

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
     * If the ghost is flagged as scared, updates the 
     * ghost's scared timer. When the timer reaches
     * certain thresholds, it updates the ghost's
     * animation state to flash, and then exit scared mode.
     */
    virtual void updateTimers(DWORD milliseconds);

    /*
     * Chooses the ghost's target tile, then updates
     * movement as normal for a Character.
     */
    virtual void updateMovement(DWORD milliseconds);

    /*
     * Updates the ghost's animation state. A ghost's
     * animation state is based on its targeting state.
     */
    virtual void updateAnimation(DWORD milliseconds);

    /*
     * Updates the ghost's target state. A ghost's
     * speed is based on its targeting state and
     * current tile type.
     */
    virtual void updateSpeed();

    /*
     * Flags the ghost for a direction reverse. Only ghosts
     * outside the ghost home may be flagged. Flagged ghosts
     * reverse direction the next time they are in the AT_TILE
     * state.
     */
    void flagForDirectionReverse();

    /*
     * Activates the ghost's CHASE targetting behavior.
     */
    void setChasing();

    /*
     * Activates the ghost's SCATTER targetting behavior.
     */
    virtual void setScatter();

    /*
     * Flags the ghost as scared and activates the ghost's
     * scared timer.
     */
    void scare();

    /*
     * Calls a helper function based on the value of isAlive.
     * If true, calls spawn(). Else, calls kill().
     */
    virtual void setLiving(bool8_t isAlive);

    /*
     * Getters for the ghost's targeting state, and some boolean values.
     */
    const GhostTargetingState getTargetingState() const;
    const bool8_t isGhostScared() const;
    const bool8_t isInGhostHome() const;

private:

    /*
     * Sets the ghost's animation state to flashing.
     */
    void flash();

    /*
     * Turns off all scared-related variables. Sets the
     * ghost's targeting state to the current state.
     */
    void unscare();

    /*
     * Sets the ghost's targeting state to DEAD so it
     * goes to the ghost home.
     */
    void kill();

    /*
     * Sets the ghost's targeting state to the current
     * state and allows the ghost to leave the ghost home.
     */
    void spawn();

    /*
     * Sets the ghost's next tile to its previous tile.
     */
    void reverseDirection();

    /*
     * Selects the neighbor of the current tile
     * that is not the ghost's previous tile.
     */
    void handleBaseTraversable();       // handler functions for selecting the 'nextTile' for various tile types

    /*
     * Selects the neighbor of the current tile that 
     * would put the ghost closest to its target.
     */
    void handleIntersection();

    /*
     * Selects the neighbor of the current tile that
     * would put the ghost closest to its target.
     * Ignores the UP direction.
     */
    void handleNoUpIntersection();

    /*
     * The ghost either goes down into the ghost home if 
     * it is marked as DEAD, or continues as though it were
     * a normally traversable tile if not marked as DEAD.
     */
    void handleGhostHomeEnter();

    /*
     * The ghost moves up out of the ghost home if alive, or
     * continues left if dead.
     */
    void handleGhostHomeExit();

    /*
     * Called to choose the next tile when at intersections. The ghost
     * chooses the neighbor that would put it physically closest to its target,
     * ignoring both its previous tile and the neighbor at 'skipDirection'.
     */
    void chooseNeighborClosestToTarget(GameUtil::BoardDirection skipDirection);

    /*
     * Chooses a random neighbor at an intersection. Ignores the ghost's previous
     * tile.
     */
    void chooseRandomNeighbor();

    /*
     * Handles the ghost's transition from AT_TILE to MOVING. Sets the ghost's
     * movement direction and next tile.
     */
    void nextTileChosen(const Tile* next, GameUtil::BoardDirection direction);

protected:

    /*
     * Virtual function. Runs the ghost's algorithm for
     * targeting pacman when in the CHASE state.
     */
    virtual void targetPacman() = 0;

    /*
     * Calls a helper function that sets the ghost's target tile
     * based on the ghost's targeting state. The target tile 
     determines the ghost's next tile when at an intersection.
     */
    void chooseTargetTile();

    /*
     * Calls a helper function that selects the ghost's next tile
     * based on the type of its current tile.
     */
    virtual void chooseNextTile();

    const Tile* mTargetTile;                // tile that ghost uses to select its next move target at intersections
    GhostTargetingState mTargetState;       // the current method of target tile selection
    GhostTargetingState mCurrentPhase;      // the current targeting phase, 
                                            //      used to give the ghost the correct targeting phase when spawning or leaving its scared state

    uint32_t mScaredTimer;

    float_t mGhostSpeed;            // ghost speeds
    float_t mGhostSpeedScared;
    float_t mGhostSpeedTunnel;

    uint32_t mScaredDuration;       // scared timings
    uint32_t mFlashThreshold;

    bool8_t mIsInGhostHome;
    bool8_t mIsScared;
    bool8_t mIsFlaggedForReverse;
};
