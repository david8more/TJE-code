#ifndef STATE_H
#define STATE_H

#include "framework.h"
#include "texture.h"
#include "game.h"
#include "mesh.h"
#include "camera.h"
#include <iostream>

using namespace std;

class StateManager;
class Mesh;
class Camera;
class Game;

class State {
public:
	//
	State();
	State( StateManager* SManager );
	virtual ~State();
	//

	virtual void init();
	virtual void render();
	virtual void update(double t);

	virtual void onKeyPressed( SDL_KeyboardEvent event );
	virtual void OnChar( WPARAM );
	virtual int stateID();

	virtual void onEnter();
	virtual void onLeave( int fut_state );

protected:
	
	void changeCurrentState( State* new_state );
	StateManager* SManager;
};

// *********************************************************************************************************
// *********************************************************************************************************

// Manager pattern

class StateManager : State {
private:
	State * current_state;

public:
	//
	StateManager();
	virtual ~StateManager();
	//

	// changing states
	void setInitialState( State* state );
	void changeCurrentState( State* new_state );
	//

	State* getCurrentState() {
		return current_state;
	}

	void init();
	void render();
	void update( double time_elapsed );
	// 'Events' function, they are simply redirected to the active state.
	void onKeyPressed( SDL_KeyboardEvent event );
	void OnChar( WPARAM wChar);
	int stateID();
};

// *********************************************************************************************************
// *********************************************************************************************************

class OptionsState : public State {
protected:
	OptionsState();
	OptionsState( StateManager* SManager );

public:

	~OptionsState();
	
	void init();
	void onEnter();
	void onLeave( int fut_state );
	void render();
	void update( double elapsed_time );
	void onKeyPressed( SDL_KeyboardEvent event );
	int stateID() { return 2; }

	// The player went up or down in the menu
	void selectionUp();
	void selectionDown();

	// The player validated the current selection
	void selectionChosen();
	
	static OptionsState* getInstance( StateManager* SManager );

	// background texture
	Texture* texture;

	// needed instances to render
	Game* game;
	Camera cam2D;
	Mesh quad;

	// BASS
	int s_sample;
	int s_channel;
	int b_channel;

	// Index of the current selected menu item
	int currentSelection;
};

#endif


