
/*
 * Managers the timer controlling fruit
 * and the list of collected fruit.
 *
 * There is an enumeration for the fruit type,
 * which acts as an offset into the fruit spritesheet
 * for the draw calls.
 *
 * Also maps fruit types to a 'collected' boolean.
 * Fruit types that are marked as collected have
 * their image drawn at the bottom of the screen.
 */
class FruitManager
{
public:

    static FruitManager* Instance();
    static const FruitManager* ConstInstace();

    /*
     * Loads the fruit spritesheet. Creates a
     * map of collected fruit.
     */
    void init();

    /*
     * Resets the fruit timer and the collected status
     * of all fruits.
     */
    void reset();

    /*
     * Resets the fruit timer so that the fruit
     * disappears when reloading the same level
     * (as when pacman dies).
     */
    void resetFromDeath();

    /*
     * Resets the fruit timer, and updates the 
     * fruit type for the next level.
     */
    void nextLevel();

    /*
     * If the fruit is active, checks if pacman has collected the fruit
     * and updates the fruit timer.
     */
    void updateFruit(DWORD milliseconds);

    /*
     * Calls helper functions for drawing the active fruit and all 
     * collected fruit.
     */
    void drawFruit();

    /*
     * Destroys the map of collected fruit.
     */
    void shutdown();

    /*
     * Increments the fruit phase. If the fruit phase is not invalid,
     * sets the fruit as active and starts the fruit timer.
     */
    void activateFruit();

    /*
     * Sets the fruit as inactive and stops the fruit timer.
     */
    void deactivateFruit();

    /*
     * Pauses the fruit timer.
     */
    void pauseFruit();

    /*
     * Unpauses the fruit timer.
     */
    void unpauseFruit();

    /*
     * Returns the current fruit phase.
     */
    const GameUtil::FruitPhase getPhase() const;

private:

    FruitManager();

    /*
     * If a fruit is active, draws the fruit at the
     * fruit tile.
     */
    void drawActiveFruit();

    /*
     * Steps through the collected fruit map. Draws any fruit 
     * marked as 'collected' at the bottom of the screen.
     */
    void drawCollectedFruit();

    static FruitManager* _instance;
    
    GameUtil::FruitType mFruitType;
    GameUtil::FruitPhase mPhase;

    typedef std::map<const GameUtil::FruitType, bool> CollectedFruit;
    typedef std::pair<const GameUtil::FruitType, bool> CollectedFruitPair;
    CollectedFruit* mCollectedFruit;

    const Tile* mFruitTile;
    Coord2D mPosition;

    int32_t mTimer;
    bool8_t mIsVisible;
    bool8_t mIsPaused;

    GLuint mFruitSpritesheetID;
};
