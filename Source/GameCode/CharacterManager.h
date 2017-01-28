
/*
 * Manages all objects that inherit from Character.
 *
 * Mostly acts as an interface through which other
 * managers may call upon Character objects.
 *
 * Though the CharacterManager also manages the timer
 * that drives the ghosts' current targeting phase.
 */
class CharacterManager
{
public:

    static CharacterManager* Instance();
    static const CharacterManager* ConstInstance();

    /*
     * Creates the character objects, and maps 
     * the ghosts to their character IDs.
     */
    void init();

    /*
     * Calls 'reset' on all characters for the new level.
     */
    void reset();

    /*
     * Calls 'resetFromDeath' on all characters to restert
     * the current level (as on pacman's death).
     */
    void resetFromDeath();

    /*
     * Calls several helper functions to update each character.
     */
    void updateCharacters(DWORD milliseconds);

    /*
     * Calls 'draw' for each character.
     */
    void drawCharacters();

    /*
     * Shutsdown then deletes each character.
     */
    void shutdown();

    /*
     * Pauses all characters.
     */
    void pauseCharacters();

    /*
     * Unpauses all characters.
     */
    void unpauseCharacters();

    /*
     * Makes all characters visible.
     */
    void makeCharactersVisible() const;

    /*
     * Makes all ghosts invisible.
     */
    void makeGhostsInvisible() const;

    /*
     * Tells pacman to play his death animation.
     */
    void playPacmanDeathAnimation() const;

    /*
     * Spawns the ghost given by 'ghostID'. This
     * is ignored if the ghost is already alive.
     */
    void spawnGhost(GameUtil::CharacterID ghostID) const;
    
    /*
     * Calls 'scare' on all ghosts. This is ignored by ghosts 
     * who are currently dead and outside the ghost home.
     */
    void scareGhosts() const;

    /*
     * Returns true if any ghost is in the scared state.
     */
    const bool8_t areGhostsScared() const;

    /*
     * Activates "Cruise Elroy" on the red ghost.
     */
    void activateCruiseElroy() const;

    /*
     * Gets the red ghost's current Cruise Elroy phase.
     */
    const GameUtil::CruiseElroyMode getCruiseElroyPhase() const;

    /*
     * Returns true if all ghosts are outside the ghost home.
     */
    const bool8_t areAllGhostsSpawned() const;

    /*
     * Returns whether the given 'character' is marked isAlive.
     */
    const bool8_t isCharacterAlive(GameUtil::CharacterID character) const;

    /*
     * Returns a const pointer to the current tile of the given 'character'.
     */
    const Tile* getCharacterCurrentTile(GameUtil::CharacterID character) const;

    /*
     * Returns the facing direction of the given 'character'.
     */
    const GameUtil::BoardDirection getCharacterFacingDirection(GameUtil::CharacterID character) const;

    /*
     * Returns the movement state of the given 'character'.
     */
    const Character::MovementState getCharacterMovementState(GameUtil::CharacterID character) const;

private:

    /*
     * Enumeration of the current ghost targeting phase.
     */
    typedef enum
    {
        SCATTER_1,
        CHASE_1,
        SCATTER_2,
        CHASE_2,
        SCATTER_3,
        CHASE_3,
        SCATTER_4,
        CHASE_4
    }GhostTimerPhase;

    CharacterManager();

    /*
     * Calls on the update for each character's timers.
     *
     * Also updates the timer that controls the ghosts'
     * targeting phase.
     */
    void updateCharacterTimers(DWORD milliseconds);

    /*
     * Calls on the update for each character's movement.
     */
    void updateCharacterMovement(DWORD milliseconds);

    /*
     * Calls on the update for each character's animation.
     */
    void updateCharacterAnimation(DWORD milliseconds);

    /*
     * Calls on the update for each character's speed.
     */
    void updateCharacterSpeed();

    /*
     * Checks collisions between pacman and ghosts. Collisions
     * occur when the current tiles of two Characters are the same.
     *
     * Kills ghosts if they collide with pacman while scared. Otherwise,
     * kills pacman and posts an event.
     */
    void updateCharacterCollision();

    /*
     * Calls kill() on 'ghost' and notifies the EventManager
     * of the foul deed.
     */
    void killGhost(Ghost* ghost);

    /*
     * Increments the ghost timer phase to the next value.
     * Then tells the ghosts to either SCATTER or CHASE based
     * on the next phase.
     */
    void toggleGhostsTargetingPhase();

    static CharacterManager* _instance;

    Character* mPacman;                                             // pacman character pointer

    typedef std::map<GameUtil::CharacterID, Ghost*> Ghosts;         // map of ghost character pointers -- so we can call on them by name
    typedef std::pair<GameUtil::CharacterID, Ghost*> IDGhostPair;
    Ghosts* mGhosts;

    GLuint mCharactersSpritesheetID;                                 // character spritesheet ID for OpenGL

    GhostTimerPhase mGhostTimerPhase;       // current timer phase, keeps us from spamming phase changes and spazzing out the ghosts

    uint32_t mPhaseTimer;
    uint32_t mScatterTheshold_2;            // targeting phase timings
    uint32_t mScatterTheshold_3;
    uint32_t mScatterTheshold_4;
    uint32_t mChaseThreshold_1;
    uint32_t mChaseThreshold_2;
    uint32_t mChaseThreshold_3;

    bool8_t mIsPaused;
};
