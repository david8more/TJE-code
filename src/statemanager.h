#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include "framework.h"
#include <iostream>

using namespace std;

// Manager pattern

class StateManager : State {
private:
	State * current_state;

public:
	//
	StateManager();
	virtual ~StateManager();
	//

	// change state
	void changeCurrentState(State* new_state);

	State* getCurrentState() {
		return current_state;
	}

	void init();
	void render();
	void update(double time_elapsed);

	// 'Events' function, they are simply redirected to the active state.
	void onKeyPressed(SDL_KeyboardEvent event);
	void OnChar(WPARAM wChar);
	int stateID();
};

#endif