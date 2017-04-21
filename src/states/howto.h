#ifndef HOWTO_H
#define HOWTO_H

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

class Howto : public State {
protected:
	Howto(StateManager* SManager);

public:

	~Howto();

	void init();
	void onEnter();
	void onLeave(int fut_state);
	void render();
	void update(double elapsed_time);
	void onKeyPressed(SDL_KeyboardEvent event);
	int stateID() { return 2; }

	static Howto* getInstance(StateManager* SManager);

	// background texture
	Texture* texture;

	// needed instances to render
	Game* game;
	Camera cam2D;
	Mesh quad;

	void selectionDown();
	void selectionUp();
	void selectionChosen();

	int currentSelection;
};

#endif