#ifndef MENUSTATE_H
#define MENUSTATE_H

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

typedef struct {
	int currents;
	int posx;
	int posy;
	int wid;
} sPositions;

typedef struct {
	float posx;
	float posy;
} sParticles;

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

	// The player went up or down in the menu
	void selectionUp();
	void selectionDown();

	// The player validated the current selection
	void selectionChosen();

	// Index of the current selected menu item
	int currentSelection;

	// BASS
	int s_sample;
	int b_sample;
	int s_channel;
	int b_channel;
	bool music_on;

	// needed instances to render

	// background texture
	Texture* texture;
	Mesh backgroundQuad;

	Game* game;
	Camera cam2D;

	Mesh quad;
	Mesh quad2;
	Texture* smokeTexture;

	Mesh loadingQuad;
	Texture* loadingTexture;

	Mesh quadSelection;
	// posiciones de cada rectangulo de selección 
	std::vector<sPositions> sel_positions;

	Mesh mParticles;
	std::vector<sParticles> vParticles;
};

#endif