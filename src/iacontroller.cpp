#include "iacontroller.h"
#include "game.h"

IAController::IAController()
{
	/*
	1100, 1100
	1100, 2100
	2100, 1100
	2100, 2100
	*/
	current_Wp = 0;

	waypoints.push_back(Vector3(1100, 500, 1100));
	waypoints.push_back(Vector3(1100, 500, 2100));
	waypoints.push_back(Vector3(2100, 500, 2100));
	waypoints.push_back(Vector3(2100, 500, 1100));
}

IAController::~IAController()
{

}

void IAController::setTarget(Vector3 target)
{
	this->target = target;
}

void IAController::update(float seconds_elapsed)
{

	// std::cout << std::endl << seconds_elapsed << std::endl;

	Game* game = Game::getInstance();

	if (!game->start)
		return;

	Airplane* controlled = player;

	double speed = seconds_elapsed * 1500;
	
	Vector3 to_target = target - controlled->getPosition();
	float distance = to_target.length();
	
	if (distance > 500)
	{
		to_target = waypoints[current_Wp] - controlled->getPosition();
		float distance_wp = to_target.length();
		
		if (distance_wp < 100)
		{
			std::cout << "next waypoint reached" << std::endl;
			current_Wp = (current_Wp + 1) % waypoints.size();
			std::cout << "changing wp to: " << current_Wp << std::endl;
		}

	}
	
	to_target.normalize();
	
	Vector3 front = controlled->model.rotateVector(Vector3(0, 0, 1));
	
	float angle = 1 - front.dot(to_target);
	Vector3 axis = to_target.cross(front);

	Matrix44 inverse_mat = controlled->model;
	inverse_mat.inverse();
	axis = inverse_mat.rotateVector(axis);

	controlled->model.rotateLocal(angle, axis);
	controlled->model.traslateLocal(0, 0, speed * seconds_elapsed * 8);
}