
/*
 * Blue ghost. Difficult to predict.
 */
class GhostBlue : public Ghost
{
public:

    GhostBlue(const Tile* spawnTile, const GameUtil::CharacterID id);
    virtual void reset();
    virtual void resetFromDeath();

protected:

    /*
     * Targets pacman. Looks ahead of pacman, then uses
     * the vector between that tile and the red ghost.
     */
    virtual void targetPacman();
};
