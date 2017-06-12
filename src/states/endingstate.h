#ifndef ENDINGSTATE_H
#define ENDINGSTATE_H

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

class EndingState : public State {
protected:
	EndingState(StateManager* SManager);

public:

	~EndingState();

	void init();
	void onEnter();
	void onLeave(int fut_state);
	void render();
	void update(double elapsed_time);
	void onKeyPressed(SDL_KeyboardEvent event);
	int stateID() { return 5; }

	static EndingState* getInstance(StateManager* SManager);

	// background texture
	Texture* texture;

	// needed instances to render
	Camera cam2D;
	Mesh quad;

};

#endif