#ifndef IACONTROLLER_H
#define IACONTROLLER_H

#include "controller.h"
#include "camera.h"

class IAController : public Controller{

private:

public:
	IAController();
	~IAController();

	bool active;
	int current_Wp = 0;

	Vector3 target;
	void update(float time_elapsed);

	void setTarget(Vector3 target);

	std::vector<Vector3> waypoints;
};

#endif // !IACONTROLLER_H
