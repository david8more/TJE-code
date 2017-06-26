#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#define SPITFIRE 0
#define P38 1
#define WILDCAT 2
#define BOMBER 3
#define FULLVIEW 0
#define CABINEVIEW 1
#define SHOOTERVIEW 2

#include "../framework.h"
#include "../texture.h"
#include "../game.h"
#include "../mesh.h"
#include "../camera.h"
#include <iostream>
#include "../bass.h"
#include "../gameentities.h"

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

	static PlayState* getInstance(StateManager* SManager) {
		static PlayState Instance(SManager);
		return &Instance;
	}

	void init();
	void onEnter();
	void render();
	void renderGUI();
	void renderWorld(Camera * camera);
	void update(double elapsed_time);
	void onLeave(int fut_state);

	void onKeyPressed(SDL_KeyboardEvent event);
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButton(SDL_MouseButtonEvent event);
	int stateID() { return 4; }
	void setView();
	int current_view;

	Game* game;
	float playTime;
	float refs_timer;

	Airplane* player;

	Vector3 viewpos;
	Vector3 viewtarget;

	// saber cuanto sumamos a la cámara dependiendo
	// del avion que usamos
	typedef struct {
		int view;
		int model;
		float qnt;
	}sTransZoom;

	bool inZoom;
	void setZoom();
	std::vector<sTransZoom> vTranslations;

	void sTransZoomCreator(int view, int model, float qnt) {
		sTransZoom toPush;
		toPush.model = model;
		toPush.view = view;
		toPush.qnt = qnt;
		vTranslations.push_back(toPush);
	}

	//GUI
	Camera cam2D;
	Mesh quad;
	std::string crosshair_tex;

	//engine sound
	bool engine_on;
};

#endif