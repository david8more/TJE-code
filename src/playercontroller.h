#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "controller.h"
#include "camera.h"

#define CONTROLLER_MODE_KEYBOARD 0
#define CONTROLLER_MODE_GAMEPAD 1

class PlayerController : public Controller{

private:
	PlayerController();
	~PlayerController();

public:

	static PlayerController* instance;

	// SINGLETON
	static PlayerController* getInstance() {
		if (instance == NULL) instance = new PlayerController();
		return instance;
	}

	void update(float time_elapsed);

	// controller actions
	int current_controller;

	void moveX(float axis, float seconds_elapsed, float speed);
	void moveXY(float Xaxis, float Yaxis, float seconds_elapsed, float speed);
	void moveY(float axis, float seconds_elapsed, float speed);
	void updateCamera(Camera * camera, float seconds_elapsed);
};

#endif // !PLAYERCONTROLLER_H
