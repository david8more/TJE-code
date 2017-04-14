#ifndef PREPLAY_H
#define PREPLAY_H

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

class PreplayState : public State {
protected:
	PreplayState(StateManager* SManager);

public:

	~PreplayState();

	void init();
	void render();
	void update(double time_elapsed);
	void onEnter();
	void onLeave();
	void onKeyPressed(SDL_KeyboardEvent event);

	int stateID() { return 3; }

	static PreplayState* getInstance(StateManager* SManager);

	// background texture
	Texture* texture;

	// needed instances to render
	Game* game;
	Camera cam2D;
	Mesh quad;

	int s_sample;
	int s_channel;

	// texture & mesh selection
	int playerModel;
	int lastRendered;;
};

#endif