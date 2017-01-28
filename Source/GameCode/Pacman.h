
/*
 * The player character class.
 */
class Pacman : public Character
{
public:

    Pacman(const Tile* spawnTile, const GameUtil::CharacterID id);
    virtual void reset();
    virtual void resetFromDeath();

    /*
     * Handles movement for pacman. Ignores movement if pacman is dead.
     */
    virtual void updateMovement(DWORD milliseconds);

    /*
     * Handles pacman's animation. Pacman does not update his animation
     * frame if he is not moving.
     */
    virtual void updateAnimation(DWORD milliseconds);

    /*
     * Updates pacman's speed. Pacman's speed is determined by whether his
     * current tile has a pellet and if the ghosts are scared.
     */
    virtual void updateSpeed();

    /*
     * Handler function that unpauses pacman and sets his animation
     * state to dead.
     */
    void playDeathAnimation();

private:

    /*
     * Handles user input. User input is only accepted if pacman
     * is both alive and unpaused.
     */
    void updateUserInput();
    
protected:

    /*
     * Selects pacman's next movement target in his facing direction.
     */
    virtual void chooseNextTile();

    float_t mPacmanSpeed;
    float_t mPacmanSpeedDots;
    float_t mPacmanSpeedScared;
    float_t mPacmanSpeedScaredDots;
};
