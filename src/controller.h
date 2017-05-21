#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "gameentities.h"

class Controller {

public:

	Controller();
	~Controller();
	Airplane* player;

	void setPlayer(Airplane* player);
	virtual void update(float time_elapsed) {}

};

#endif