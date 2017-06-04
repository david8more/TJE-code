#include "iacontroller.h"
#include "World.h"
#include "game.h"
#include "framework.h"

IAController::IAController()
{
	current_Wp = 0;
	state = "";
	totalWP = 50;

	Vector3 c(2000, -10, 1700);

	float radius = 670.0;

	for (float i = 0; i < 180 * DEG2RAD * totalWP; i += 10 * DEG2RAD)
	{
		float fx = random() * 500;
		float fy = random() * 200;
		float fz = random() * 500;

		float x = c.x + cos(i) * radius + fx;
		float y = 400.0 + fy;
		float z = c.z + sin(i) * radius + fz;

		waypoints.push_back(Vector3(x, y, z));
	}
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

	// player info
	Airplane* playerAir = World::getInstance()->playerAir;
	Vector3 playerPos = playerAir->getPosition();
	//

	double speed = World::instance->playerAir->speed;

	state = "chasing";

	Vector3 to_target = target - controlled->getPosition();
	Vector3 targetToPlayer = to_target;

	// distance to target
	float distance = to_target.length();

	// waypoints: si estamos lejos, si tiene poca vida o si tiene que bajar más de la cuenta

	if (distance > 500.0 || controlled->life < 50.0 || controlled->getPosition().y < 475.0)
	{
		state = "waypoints";

		to_target = waypoints[current_Wp] - controlled->getPosition();
		float distance_wp = to_target.length();

		if (distance_wp < 100.0)
		{
			current_Wp = (current_Wp + 1) % waypoints.size();
		}

	}

	if (controlled->life < 50.0)
	{
		state = "retiring";
		speed *= 1.5;
		controlled->model.traslateLocal(0, 0, speed * seconds_elapsed);
		return;
	}


	// rotaciones

	if (to_target.length())
		to_target.normalize();

	Vector3 front = controlled->model.rotateVector(Vector3(0, 0, 1));

	float angle = 1 - front.dot(to_target);
	float angleWithPlayer = 1 - front.dot(targetToPlayer.normalize());

	Vector3 axis = to_target.cross(front);

	Matrix44 inverse_mat = controlled->model;
	inverse_mat.inverse();
	axis = inverse_mat.rotateVector(axis);

	// orient

	Vector3 up(0, 1, 0);
	Vector3 localUp = inverse_mat.rotateVector(up);
	localUp.z = 0;
	localUp.normalize();
	float r = localUp.dot(Vector3(0, 1, 0));
	Vector3 axisUp = localUp.cross(Vector3(0, 1, 0));

	if (axisUp.z > 0)
	{
		controlled->model.rotateLocal((1.0 - r) * seconds_elapsed, Vector3(0, 0, 1));
	}
	else
	{
		controlled->model.rotateLocal((1.0 - r) * seconds_elapsed, Vector3(0, 0, -1));
	}

	controlled->model.rotateLocal(angle * seconds_elapsed * 5, axis);
	controlled->model.traslateLocal(0, 0, speed * seconds_elapsed);

	if (distance < 300.0 && (abs(angleWithPlayer) > 0.05))
	{
		controlled->shoot();
	}
}