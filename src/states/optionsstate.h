#ifndef OPTIONSSTATE_H
#define OPTIONSSTATE_H

#define MUSIC 0
#define MUSIC_VOL 1
#define EFFECTS 2
#define FULLSCREEN 3
#define GAMEMODE 4
#define FRIENDLYFIRE 5

#include "../framework.h"
#include "../texture.h"
#include "../game.h"
#include "../mesh.h"
#include "../camera.h"
#include "../bass.h"
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

	// Index of the current selected menu item
	int currentSelection;

	void upVol();
	void downVol();
};

#endif


