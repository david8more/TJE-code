#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "framework.h"
#include "texture.h"
#include "game.h"
#include "mesh.h"
#include "camera.h"
#include <iostream>

using namespace std;

class StateManager;
class Entity;
class EntityMesh;
class Mesh;
class Camera;
class Game;

class PlayState : public State {
protected:

	PlayState(StateManager* SManager);

public:

	~PlayState();
	static PlayState* getInstance(StateManager* SManager);

	void init();
	void onEnter();
	void render();
	void update(double elapsed_time);
	void onLeave(int fut_state);

	void onKeyPressed(SDL_KeyboardEvent event);
	void onKeyUp(SDL_KeyboardEvent event);
	int stateID() { return 4; }

	Vector3 viewpos;
	Vector3 viewtarget;

	//GUI
	Camera cam2D;
	Mesh quad;
	Texture * crosshair_tex;

	// background music
	int b_sample;
	int b_channel;

	//engine sound
	bool engine_on;
	int e_sample;
	int e_channel;
};

#endif