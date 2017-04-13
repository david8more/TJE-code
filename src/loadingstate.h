#ifndef LOADINGSTATE_H
#define LOADINGSTATE_H

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

class LoadingState : public State {
protected:
	LoadingState(StateManager* SManager);

public:

	~LoadingState();

	void init();
	void render();
	void update(double time_elapsed);
	void onEnter();
	void onLeave();
	void onKeyPressed(SDL_KeyboardEvent event);

	int stateID() { return -1; }

	static LoadingState* getInstance(StateManager* SManager);

	bool loaded;

	// background texture
	Texture* texture;

	// needed instances to render
	Game* game;
	Camera cam2D;
	Mesh quad;
};

#endif