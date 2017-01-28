
namespace GameUtil
{
    /*
     * Static class for some helpful utility functions.
     */
    class Utilities
    {
    public:

        /*
         * Returns the square-distance between two (x,y) points.
         */
        static float squareDistance(const float x1, const float x2, const float y1, const float y2);

        /*
         * Draws a sprite with OpenGL coordinates given by 
         * 'vertTR' (top-right) and 'vertBL' (bottom-left),
         * and texture coordinates starting at 'textureCoord'.
         *
         * This function just binds the verts and texture coords
         * together.
         */
        static void drawSprite(const Coord2D& vertTR, const Coord2D& vertBL, const Coord2D& textureCoord, const Coord2D& textureSize);
    };

    /*
     * Enumeration of character IDs.
     *
     * Used in getter functions on the CharacterManager,
     * and to get ghost home tiles and spawn points from
     * the BoardManager.
     */
    enum CharacterID
    {
        PACMAN,

        GHOST_RED,
        GHOST_PINK,
        GHOST_BLUE,
        GHOST_ORANGE,
    };

    /*
     * Enumeration of animation IDs.
     *
     * Used to calculate the texture coordinates
     * of animated characters in the character spritesheet.
     */
    enum AnimChunkID
    {
        ANIM_PACMAN,

        ANIM_GHOST_RED,
        ANIM_GHOST_PINK,
        ANIM_GHOST_BLUE,
        ANIM_GHOST_ORANGE,

        ANIM_GHOST_SCARED,
        ANIM_GHOST_FLASHING,

        ANIM_GHOST_DEAD,
        ANIM_PACMAN_DEAD
    };

    /*
     * Enumeration of facing directions.
     *
     * Used for character facing directions and
     * tile neighbor-mappings.
     *
     * With this, a character can call on their next tile
     * by whichever way they are currently facing.
     */
    enum BoardDirection
    {
        UP,
        LEFT,
        DOWN,
        RIGHT,

        INVAL
    };

    /*
     * Enumeration of Cruise Elroy states that the
     * red ghost could be in.
     *
     * This is so the red ghost can only be enhanced
     * twice during a level.
     */
    enum CruiseElroyMode
    {
        NONE,
        CRUISE_1,
        CRUISE_2,
        CRUISE_INVAL
    };

    /*
     * Enumeration of fruit phases.
     *
     * This is so the fruits only ever are displayed
     * twice during a level.
     */
    enum FruitPhase
    {
        PRE_FRUIT,
        FRUIT_1,
        FRUIT_2,
        FRUIT_INVAL
    };

    /*
     * Enumeration of fruit types.
     *
     * Used to offset the texture coordinates in the fruit
     * spritesheet.
     *
     * Points values for collected fruit are based on
     * the fruit type of the current level.
     */
    enum FruitType
    {
        FRUIT_KEY,
        FRUIT_BELL,
        FRUIT_GALAXIAN,
        FRUIT_GRAPES,
        FRUIT_APPLE,
        FRUIT_PEACH,
        FRUIT_STRAWBERRY,
        FRUIT_CHERRY,

        FRUIT_TYPE_INVAL
    };

    /*
     * Enumeration of UI states.
     *
     * Used to determine which helper functions the 
     * UIManager should call to draw.
     */
    enum UIState
    {
        UI_MENU,
        UI_PREGAME_1,
        UI_PREGAME_2,
        UI_GAME
    };
}

