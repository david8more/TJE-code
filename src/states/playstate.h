#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#define SPITFIRE 0
#define P38 1
#define WILDCAT 2
#define BOMBER 3
#define FULLVIEW 0
#define CABINEVIEW 1

#include "../framework.h"
#include "../texture.h"
#include "../game.h"
#include "../mesh.h"
#include "../camera.h"
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

	static PlayState* getInstance(StateManager* SManager) {
		static PlayState Instance(SManager);
		return &Instance;
	}

	void init();
	void onEnter();
	void render();
	void update(double elapsed_time);
	void renderHUD();
	void onLeave(int fut_state);

	void onKeyPressed(SDL_KeyboardEvent event);
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButton(SDL_MouseButtonEvent event);
	int stateID() { return 4; }
	void setView();

	Vector3 viewpos;
	Vector3 viewtarget;
	int current_view;

	// saber cuanto sumamos a la cámara dependiendo
	// del avion que usamos
	typedef struct {
		int view;
		int model;
		float qnt;
	}sTransZoom;

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

	// background music
	int b_sample;
	int b_channel;

	//engine sound
	bool engine_on;
	int e_sample;
	int e_channel;

	// m60 attributes
	float timer;
	bool shooting;
	bool overused;
	int shootingtime;
	float cadencia;
};

#endif