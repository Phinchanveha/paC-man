
/*
 * In order to preserve const-correctness, each manager class
 * has an Instance() method and a ConstInstance() method.
 *
 * ConstInstance() returns a const pointer to the singleton object.
 * This way, objects can't call things on managers that they shouldn't
 * (such as update or draw).
 *
 * It is fine for objects to call on managers for const getter functions,
 * but if an object needs to affect another object, they should post an
 * event to the EventManager (objects may affect objects downward, but
 * not upwards or sideways).
 *
 * The EventManager enqueues events, and calls on the appropriate handler
 * functions on these objects' behalf.
 *
 * Keep in mind that the order of events posted is important. Some events
 * (such as the beginning and ending of levels) clears the EventManager's
 * queues, which means any events posted after are not handled.
 */
class EventManager
{
public:

    /*
     * Enumeration of all gameplay-related events.
     */
    typedef enum
    {
        EVENT_INVAL,

        START_FROM_MENU,        // game flow events -- resetting    // resets the game for level 1
        START_FROM_GAME_WIN,                                        // resets the game for the next level, begins the start level timer
        START_FROM_GAME_LOSE,                                       // resets the game for restarting the current level, begins the start level timer (reduced time)
        LEVEL_WIN,              // game flow events -- ending game  // level won, begins the end level timer
        LEVEL_LOSE,                                                 // level lost, begins the end level timer

        GAME_OVER,              // ui events                        // transitions from the game back to the menu
        UI_ENTER_PREGAME_2,                                         // tells the UIManager to enter its second pre-game state
        UI_ENTER_GAME,                                              // tells the UIManager to only display score and lives

        PAUSE,                  // pause and unpause events         // pauses characters and fruit
        UNPAUSE,                                                    // unpauses characters and fruit

        CHARACTERS_VISIBLE,     // visibility events                // makes characters visible
        GHOSTS_INVISIBLE,                                           // makes ghosts invisible

        SPAWN_GHOST_RED,        // spawn ghost events               // spawns ghosts of different colors
        SPAWN_GHOST_PINK,
        SPAWN_GHOST_BLUE,
        SPAWN_GHOST_ORANGE,

        SCARE_GHOSTS,           // ghost events                     // scares ghosts
        CRUISE_ELROY,                                               // activates the next level of Cruise Elroy
        GHOST_KILLED,                                               // awards points, starts the short pause whenever a ghost is killed

        ACTIVATE_FRUIT,         // fruit event                      // makes fruit visible and starts its timer

        PLAY_PACMAN_DEATH_ANIM, // animation events                 // tells pacman to die
        PLAY_TILE_FLASH,                                            // tells tiles to flash enthusiastically

        CONSUME_LIFE,           // stats events                     // reduces life counter by 1
        POINTS_PELLET,                                              // points for pellet
        POINTS_POWER_PELLET,                                        // points for power pellet
        POINTS_KILL_BONUS,                                          // points for perfect round bonus
        POINTS_FRUIT                                                // points for fruit
    }GameEvent;

    /*
     * Enumaration of sound events.
     */
    typedef enum
    {
        SOUND_INVAL,

        SOUND_START,
        SOUND_EXTRA_LIFE,
        SOUND_PELLET,
        SOUND_FRUIT,
        SOUND_PACMAN_DEATH,
        SOUND_GHOST_DEATH
    }SoundEvent;

    static EventManager* Instance();
    static const EventManager* ConstInstace();
    void init();

    /*
     * Clears the queues.
     */
    void reset();

    /*
     * Calls on handler functions for each of the EventManager's queues.
     */
    void update();

    /*
     * Clears, then destroys the queues.
     */
    void shutdown();

    /*
     * Enqueues a game event.
     */
    void postEvent(GameEvent eventType) const;

    /*
     * Enqueues a sound event.
     */
    void postSoundEvent(SoundEvent eventType) const;

private:

    EventManager();

    /*
     * Handles all enqueued game events posted
     * since the last update frame.
     */
    void handleGameEventQueue();

    /*
     * Handles all enqueued sound events posted
     * since the last update frame.
     */
    void handleSoundsEventQueue();

    static EventManager* _instance;

    typedef std::queue<GameEvent> EventQueue;
    EventQueue* mEventQueue;

    typedef std::queue<SoundEvent> SoundQueue;
    SoundQueue* mSoundQueue;
};
