#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "gameentities.h"
#include "camera.h"

#define CONTROLLER_MODE_KEYBOARD 0
#define CONTROLLER_MODE_GAMEPAD 1

class PlayerController {
private:
	PlayerController();
	~PlayerController();
public:
	Airplane* player;
	int current_controller;

	static PlayerController* instance;

	// SINGLETON
	static PlayerController* getInstance() {
		if (instance == NULL) instance = new PlayerController();
		return instance;
	}
	void setPlayer(Airplane* player);
	void update(float time_elapsed);

	// controller actions
	void moveX(float axis, float seconds_elapsed, float speed);
	void moveXY(float Xaxis, float Yaxis, float seconds_elapsed, float speed);
	void moveY(float axis, float seconds_elapsed, float speed);
	void shoot();
	void updateCamera(Camera * camera, float seconds_elapsed);
};

#endif // !PLAYERCONTROLLER_H
