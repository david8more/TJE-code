#ifndef OPTIONSSTATE_H
#define OPTIONSSTATE_H

#define MUSIC 0
#define EFFECTS 1
#define FULLSCREEN 2
#define GAMEMODE 3

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
	void onKeyUp(SDL_KeyboardEvent event);
	int stateID() { return 1; }

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


