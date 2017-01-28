
/*
 * Manages timers that control game flow during
 * pauses at the beginning and ending of a level.
 *
 * The EventManager will pause the game and activate 
 * different timers on this object for different events.
 *
 * The activated timers count down, and at certain
 * thresholds post new events to the EventManager.
 *
 * Such events could be changing the UIManager state,
 * playing an animation, or unpausing the game to begin play.
 */
class TimerManager
{
public:

    static TimerManager* Instance();
    static const TimerManager* ConstInstace();
    void init();

    /*
     * Resets all timers to 0 and boolean flags
     * to false.
     */
    void reset();

    /*
     * Calls the handler function for any active timers. 
     */
    void updateTimers(DWORD milliseconds);
    void shutdown();

    /*
     * Activates the start game timer. Flags the timer to
     * consume a life.
     */
    void activateStartTimerFromMenu();

    /*
     * Activates the start game timer. The duration of this
     * timer is halved.
     */
    void activateStartTimerFromWin();

    /*
     * Activates the start game timer. Flags the timer to
     * consume a life. The duration of this timer is halved.
     */
    void activateStartTimerFromLoss();

    /*
     * Activates the end game timer. Flags the timer to
     * handle "win" condition.
     */
    void activateEndGameTimerWin();

    /*
     * Activates the end game timer. Flags the timer to
     * handle "loss" condition.
     */
    void activateEndGameTimerLoss();

    /*
     * Activates the ghost dead timer.
     */
    void activateDeadGhostTimer();

private:

    TimerManager();

    /*
     * Handles the countdown for a short pause before the start
     * of play. 
     *
     * If flagged to consume a life, it posts a CONSUME_LIFE
     * event.
     *
     * At the end, it posts an UNPAUSE event.
     */
    void updateStartGameTimer(DWORD milliseconds);

    /*
     * Handles the countdown for a short pause at the end of
     * play.
     *
     * If flagged as a loss, it posts an event to play pacman's
     * death animation when half over. When finished, it posts
     * an event to restart the same level.
     *
     * If not flagged as a loss, it posts an event to play the
     * tile flash animation when half over. When finished, it
     * posts an event to start the next level.
     */
    void updateEndGameTimer(DWORD milliseconds);

    /*
     * Handles the countdown for the short pause when a ghost
     * is eaten. Posts an UNPAUSE event when finished.
     */
    void updateDeadGhostTimer(DWORD milliseconds);

    static TimerManager* _instance;

    int32_t mStartGameTimer;            // timers
    int32_t mEndGameTimer;
    int32_t mDeadGhostTimer;

    bool8_t mHalfPauseTimePassed;       // game flow control booleans
    bool8_t mWillConsumeLife;
    bool8_t mDidLose;
};
