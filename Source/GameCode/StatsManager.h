
/*
 * Manages the current level, score, highscore,
 * and stats for speed and timing.
 *
 * Character speed and timing values change
 * based on the current level number. 
 *
 * Each time the level changes, many objects 
 * request these updated stats and save them so 
 * they don't spam request the same value.
 */
class StatsManager
{
public:

    static StatsManager* Instance();
    static const StatsManager* ConstInstace();
    void init();

    /*
     * Resets the level counter and score for a new game.
     */
    void reset();

    /*
     * Calls writeHighScoreFile() to prep for game end.
     */
    void shutdown();

    /*
     * Reads the highscore file. If no file is present,
     * it creates a highscore file.
     */
    void readHighScoreFile();

    /*
     * Writes the highscore out to a file.
     */
    void writeHighScoreFile();

    /*
     * Increments the level counter and updates stats
     * values.
     */
    void nextLevel();

    /*
     * Decrements the life counter.
     */
    void consumeLife();

    /*
     * Updates the score for a collected pellet.
     */
    void pelletCollected();

    /*
     * Updates the score for a collected power pellet,
     * and resets the counter for ghost kills.
     */
    void powerPelletCollected();

    /*
     * Updates the score for a killed ghost. The amount
     * added increases as more ghosts are killed.
     */
    void ghostKilled();

    /*
     * Updates the score with a large bonus if all ghosts
     * have been killed at all power pellets this level.
     */
    void ghostKillBonusPoints();

    /*
     * Updates the score for a collected fruit. The amount
     * added is determined by the enumerated fruit type.
     */
    void fruitCollected();

    /*
     * Returns the highscore.
     */
    const int32_t getHighScore() const;

    /*
     * Returns the score.
     */
    const int32_t getScore() const;

    /*
     * Returns the current number of lives.
     */
    const int32_t getLives() const;

    /*
     * Returns the fruit type for the current level.
     */
    const GameUtil::FruitType getFruitType() const;

    /*
     * Getter functions for various pacman speeds.
     */
    const float_t getPacmanSpeed() const;
    const float_t getPacmanSpeedDots() const;
    const float_t getPacmanScaredSpeed() const;
    const float_t getPacmanScaredSpeedDots() const;

    /*
     * Getter functions for various ghost speeds.
     */
    const float_t getGhostSpeed() const;
    const float_t getGhostSpeedScared() const;
    const float_t getGhostSpeedTunnel() const;

    /*
     * Getter functions for Cruise Elroy speeds and dots remaining triggers.
     */
    const float_t getCruiseElroySpeed_1() const;
    const float_t getCruiseElroySpeed_2() const;
    const float_t getCruiseElroyDots_1() const;
    const float_t getCruiseElroyDots_2() const;

    /*
     * Getter functions for ghost timers.
     */
    const int32_t getGhostScaredTime() const;
    const int32_t getGhostFlashTime() const;

    /*
     * Returns the timer threshold to enter the given ghost 
     * targeting phase.
     */
    const int32_t getGhostPhaseTime(int32_t phase) const;

private:

    StatsManager();

    /*
     * Adds 'points' to score, and updates the highscore
     * if necessary.
     */
    void updateScore(const int32_t points);

    static StatsManager* _instance;

    int32_t mLevel;
    int32_t mLives;
    int32_t mScore;
    int32_t mHighScore;
    int32_t mPrevScore;

    int32_t mTableIndex;

    int32_t mGhostsKilledOnPellet;
    int32_t mGhostKillCount;

    // STATS

    float_t mPacmanSpeed;           // pacman speeds
    float_t mPacmanSpeedDots;
    float_t mPacmanSpeedScared;
    float_t mPacmanSpeedScaredDots;

    float_t mGhostSpeed;            // ghost speeds
    float_t mGhostSpeedScared;
    float_t mGhostSpeedTunnel;

    float_t mCruiseElroySpeed;      // cruise elroy stats
    float_t mCruiseElroyDots;

    // STATS TABLES -- some stats don't follow a pattern and are kept in a table

    const GameUtil::FruitType FRUIT_TYPE_TABLE[21] =
    {
        GameUtil::FruitType::FRUIT_CHERRY,      GameUtil::FruitType::FRUIT_STRAWBERRY,  GameUtil::FruitType::FRUIT_PEACH,      
        GameUtil::FruitType::FRUIT_PEACH,       GameUtil::FruitType::FRUIT_APPLE,       GameUtil::FruitType::FRUIT_APPLE,      
        GameUtil::FruitType::FRUIT_GRAPES,      GameUtil::FruitType::FRUIT_GRAPES,      GameUtil::FruitType::FRUIT_GALAXIAN,   
        GameUtil::FruitType::FRUIT_GALAXIAN,    GameUtil::FruitType::FRUIT_BELL,        GameUtil::FruitType::FRUIT_BELL,       
        GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY,        
        GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY,        
        GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY,         GameUtil::FruitType::FRUIT_KEY
    };
    
    const int32_t GHOST_SCARED_TIMES[21] = 
    {
        6000, 5000, 4000, 3000, 2000, 5000, 2000, 
        2000, 1000, 5000, 2000, 1000, 1000, 3000, 
        1000, 1000, 0, 1000, 0, 0, 0
    };

    const int32_t GHOST_PHASE_TIMES_1[8] = { 84000, 77000, 57000, 50000, 30000, 25000, 5000, 0 };
    const int32_t GHOST_PHASE_TIMES_2[8] = { 1093000, 1086000, 1066000, 1059000, 1039000, 1034000, 16, 0 };
    const int32_t GHOST_PHASE_TIMES_3[8] = { 1095000, 1088000, 1068000, 1063000, 1043000, 1038000, 16, 0 };
};
