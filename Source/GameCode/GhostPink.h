
/*
 * Pink ghost. Cuts pacman off.
 */
class GhostPink : public Ghost
{
public:

    GhostPink(const Tile* spawnTile, const GameUtil::CharacterID id);
    virtual void reset();
    virtual void resetFromDeath();

protected:

    /*
     * Targets pacman. Targets several spaces ahead of pacman.
     */
    virtual void targetPacman();
};
