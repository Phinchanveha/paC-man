
/*
 * Orange ghost. Wanders off when near pacman.
 */
class GhostOrange : public Ghost
{
public:

    GhostOrange(const Tile* spawnTile, const GameUtil::CharacterID id);
    virtual void reset();
    virtual void resetFromDeath();

protected:

    /*
     * Targets pacman. Targets pacman directly when far.
     * Targets the ghost's home corner when near pacman.
     */
    virtual void targetPacman();
};
