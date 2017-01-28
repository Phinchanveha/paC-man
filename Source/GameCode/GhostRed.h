
/*
 * Red ghost. Aggressivly chases pacamn.
 */
class GhostRed : public Ghost
{
public:

    GhostRed(const Tile* spawnTile, const GameUtil::CharacterID id);
    virtual void reset();
    virtual void resetFromDeath();

    /*
     * Updates the red ghost's speed with special rules for when
     * the red ghost is in "Cruise Elroy" mode.
     */
    virtual void updateSpeed();

    /*
     * Sets the red ghost's scatter behavior. The red ghost ignores
     * scatter behavior when "Cruise Elroy" is activated.
     */
    virtual void setScatter();

    /*
     * Activates the red ghost's next level of "Cruise Elroy" if able.
     */
    void activateCruiseElroy();

    /*
     * Getter for the red ghost's "Cruise Elroy" mode.
     */
    const  GameUtil::CruiseElroyMode getCruiseElroyPhase() const;

protected:

    /*
    * Targets pacman. Targets pacman directly.
    */
    virtual void targetPacman();

    GameUtil::CruiseElroyMode mCruiseElroyMode;

    float_t mCruiseElroySpeed_1;
    float_t mCruiseElroySpeed_2;
};
