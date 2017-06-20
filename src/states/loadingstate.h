#ifndef LOADINGSTATE_H
#define LOADINGSTATE_H

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

class LoadingState : public State {
protected:
	LoadingState(StateManager* SManager);

public:

	~LoadingState();

	void onEnter();
	void onLeave(int fut_state);
	void render();
	void update(double time_elapsed);
	void onKeyPressed(SDL_KeyboardEvent event);
	int stateID() { return -1; }

	static LoadingState* getInstance(StateManager* SManager) {
		static LoadingState Instance(SManager);
		return &Instance;
	}

	void Load();

	// background texture
	Texture* texture;
	Mesh backgroundQuad;

	Game* game;
	Camera cam2D;
	
	float current_ttl;
	int iterator;
	std::vector<std::string> slider;

};

#endif