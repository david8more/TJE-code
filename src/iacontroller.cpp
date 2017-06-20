#include "iacontroller.h"
#include "World.h"
#include "game.h"
#include "framework.h"
#include <algorithm>

IAController::IAController()
{
	current_Wp = 0;
	state = "";
	totalWP = 50;

	Vector3 c(2000, -10, 1700);

	float radius = 500.0;

	float order = (random() * 50)  * 10 * DEG2RAD;

	for (float i = order; i < 180 * DEG2RAD * (totalWP + order); i += 10 * DEG2RAD)
	{
		float fx = random() * 500;
		float fy = random() * 200;
		float fz = random() * 500;

		float x = c.x + cos(i) * radius + fx;
		float y = 300.0 + fy;
		float z = c.z + sin(i) * radius + fz;

		waypoints.push_back(Vector3(x, y, z));
	}

	//std::random_shuffle(waypoints.begin(), waypoints.end());
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
	Game* game = Game::getInstance();

	if (!game->start)
		return;

	Airplane* controlled = player;

	//
	Airplane* playerAir = World::getInstance()->playerAir;
	Vector3 playerPos = playerAir->getPosition();
	double speed = World::instance->playerAir->speed;
	Vector3 targetToPlayer = target - controlled->getPosition();
	//

	// any target
	Vector3 to_target;
	state = "waypoints";

	// waypoints: tb si tiene poca vida o si tiene que bajar más de la cuenta
	to_target = waypoints[current_Wp] - controlled->getPosition();
	float distance_wp = to_target.length();

	if (distance_wp < 100.0)
	{
		current_Wp = (current_Wp + 1) % waypoints.size();
	}

	if (controlled->life < 50.0)
	{
		state = "retiring";
		speed *= 1.5;
	}

	// distance to player
	float distanceToPlayer = targetToPlayer.length();

	if (distanceToPlayer < 80.0)
	{
		state = "avoiding collision";
	}

	else if (distanceToPlayer < 750.0 && state != "retiring")
	{
		if (distance_wp < 1000)
		{
			to_target = targetToPlayer;
			state = "chasing";

		}
		else
		{
			state = "waypoints";
		}
	}

	// rotaciones

	if (to_target.length())
		to_target.normalize();

	Vector3 front = controlled->model.rotateVector(Vector3(0, 0, 1));
	float angle = 1 - front.dot(to_target);
	Vector3 axis = to_target.cross(front);

	Matrix44 inverse_mat = controlled->model;
	inverse_mat.inverse();
	axis = inverse_mat.rotateVector(axis);

	controlled->model.rotateLocal(angle * seconds_elapsed * 5, axis);
	controlled->model.traslateLocal(0, 0, speed * seconds_elapsed);

	// orient

	Vector3 up(0, 1, 0);
	Vector3 localUp = inverse_mat.rotateVector(up);
	localUp.z = 0;
	localUp.normalize();
	float r = localUp.dot(Vector3(0, 1, 0));
	Vector3 axisUp = localUp.cross(Vector3(0, 1, 0));

	if (axisUp.z > 0)
		controlled->model.rotateLocal((1.0 - r) * seconds_elapsed, Vector3(0, 0, 1));
	else
		controlled->model.rotateLocal((1.0 - r) * seconds_elapsed, Vector3(0, 0, -1));
	
	if (state != "chasing" || distanceToPlayer > 300.0)
		return;

	float angleWithPlayer = 1 - front.dot(targetToPlayer.normalize());
	
	if (abs(angleWithPlayer) > 0.05)
	{
		state = "shooting";
		controlled->shoot();
	}
}