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
	virtual void onKeyUp(SDL_KeyboardEvent event);
	virtual void onMouseButton(SDL_MouseButtonEvent event);
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
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButton(SDL_MouseButtonEvent event);
	void OnChar( WPARAM wChar);
	int stateID();
};

#endif


