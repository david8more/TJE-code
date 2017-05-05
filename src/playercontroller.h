#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "gameentities.h"
#include "camera.h"

class PlayerController {
private:
	PlayerController();
	~PlayerController();
public:
	Airplane* player;

	static PlayerController* instance;

	// SINGLETON
	static PlayerController* getInstance() {
		if (instance == NULL) instance = new PlayerController();
		return instance;
	}
	void setPlayer(Airplane* player);
	void update(float time_elapsed);
};

#endif // !PLAYERCONTROLLER_H
