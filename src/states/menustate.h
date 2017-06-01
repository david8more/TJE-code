#ifndef MENUSTATE_H
#define MENUSTATE_H

#define PLAYSTATE 0
#define HOWTOPLAY 1
#define OPTIONS 2

#include "../framework.h"
#include "../texture.h"
#include "../game.h"
#include "../mesh.h"
#include "../camera.h"
#include <iostream>

using namespace std;

class StateManager;
class Mesh;
class Camera;
class Game;

class MenuState : public State {
protected:
	MenuState(StateManager* SManager);

public:

	~MenuState();

	void init();
	void onEnter();
	void onLeave(int fut_state);
	void render();
	void update(double elapsed_time);
	void onKeyPressed(SDL_KeyboardEvent event);
	int stateID() { return 0; }

	static MenuState* getInstance(StateManager* SManager) {
		static MenuState Instance(SManager);
		return &Instance;
	}

	bool inCurrentSelection(int selection);

	// The player went up or down in the menu
	void selectionUp();
	void selectionDown();

	// The player validated the current selection
	void selectionChosen();

	// Index of the current selected menu item
	int currentSelection;

	bool music_on;

	// needed instances to render

	// background texture
	Texture* texture;
	Mesh backgroundQuad;

	Game* game;
	Camera cam2D;

	// para representar la current selection necesitamos 4 quads y 8 texturas distintas

	Mesh PG_QUAD;
	Mesh HT_QUAD;
	Mesh OPT_QUAD;
	Mesh E_QUAD;

	// pra las texturas la cogeremos al usarla

	Texture* SELECTION_TEX;
};

#endif