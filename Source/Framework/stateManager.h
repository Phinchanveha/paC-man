class StateManagerC
{
public:
	static StateManagerC	*CreateInstance();
	static StateManagerC	*GetInstance() {return sInstance;};
	~StateManagerC(){};

	enum 
	{
		NO_BALLS_FILLED,
		HALF_BALLS_FILLED,
		ALL_BALLS_FILLED
	};

	int32_t		getState();
	void		setState(int32_t stateToSet);
	void		shutdown();

    typedef enum
    {
        IN_MENU,
        IN_GAME
    } ApplicationState;

    ApplicationState getApplicationState() const;
    void setApplicationState(const ApplicationState newState);

private:
	static StateManagerC *sInstance;
	StateManagerC(){};
	int32_t		mCurrentState;

    ApplicationState mCurrentApplicationState;
};