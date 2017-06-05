#ifndef SELECTIONSTATE_H
#define SELECTIONSTATE_H

#include "../framework.h"
#include "../texture.h"
#include "../game.h"
#include "../mesh.h"
#include "../camera.h"
#include <iostream>
#include <cmath>
#include <ctime>
#include <vector>

using namespace std;

class StateManager;
class Mesh;
class Camera;
class Game;

class SelectionState : public State {
protected:
	SelectionState(StateManager* SManager);

public:

	~SelectionState();

	void init();
	void render();
	void update(double time_elapsed);
	void onEnter();
	void onLeave();
	void onKeyPressed(SDL_KeyboardEvent event);

	int stateID() { return 3; }

	void selectionDown();
	void selectionUp();
	void selectionChosen();

	static SelectionState* getInstance(StateManager* SManager);

	// background texture
	Texture* texture;

	// needed instances to render
	Game* game;
	Camera cam2D;
	Mesh quad;

	// texture & mesh selection
	int playerModel;
	int lastRendered;

	std::vector<std::vector<std::string>> selectionHelp;
};

#endif