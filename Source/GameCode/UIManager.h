
/*
 * Manages UI elements.
 *
 * Displays text for score, highscore, 
 * and lives counter.
 */
class UIManager
{
public:

    static UIManager* Instance();
    static const UIManager* ConstInstace();

    /*
     * Loads UI images. Initializes the score strings.
     */
    void init();

    /*
     * Resets the score string for a new game.
     */
    void reset();

    /*
     * Listens for the user to press 'spacebar' when in menu
     * to begin the game.
     */
    void update();

    /*
     * Calls helper functions to draw UI elements for each
     * UI state.
     */
    void draw();
    void shutdown();

    /*
     * Requests the score and highscore from the StatsManager.
     * Calls a helper function to recursively convert the integers
     * into a string that can be displayed through text.
     */
    void updateScoreStrings();

    /*
     * Sets the UI state that controls which helper function is
     * called from draw().
     */
    void setUIState(const GameUtil::UIState state);

private:

    UIManager();

    /*
     * Draws the game over menu with the score and highscore.
     */
    void drawGameOverScreen();

    /*
     * Draws a blue "PLAYER ONE" for the pregame UI.
     */
    void drawPlayerOneText();

    /*
     * Draws a yellow "READY" for the pregame UI.
     */
    void drawReadyText();

    /*
     * Draws score and high score, and the remaining lives symbols.
     */
    void drawGameUI();

    /*
     * Draws 'text' of 'length' as a series of sprites starting at (x,y).
     */
    void drawText(const char8_t* text, const int32_t length, const int32_t x, const int32_t y);

    /*
     * For each life remaining, draws a life symbol.
     */
    void drawLives();

    /*
     * Recursively converts each digit of 'num' into a character
     * and places it in the 'buffer' at 'index'.
     */
    void updateScoreStringHelper(char8_t* buffer, int32_t num, int32_t& index);

    static UIManager* _instance;

    GameUtil::UIState mCurrentState;

    std::string mScoreString;
    std::string mHighScoreString;

    GLuint mMenuImageID;
    GLuint mTextSpritesheetID;
    GLuint mLivesSpritesheetID;
};
