#ifndef GAMEENTITIES_H
#define GAMEENTITIES_H

#include "entity.h"

class IAController;

class Airplane : public EntityCollider {
public:

	Airplane(int model, IAController* controller, bool culling = true);
	~Airplane();

	IAController* controller;

	int planeModel;

	bool engine;
	void engineOnOff();

	double wheels_rotation;

	int life;
	int torpedosLeft;
	float cadence;
	float damageM60;
	float damageMissile;

	// m60 attributes
	float timer;
	bool overused;
	int shootingtime;
	bool shooting;

	Torpedo* torpedos[2];

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void shoot();
	void createTorpedos();
	void torpedoShoot();
	void onCollision(EntityCollider* collided_with);
};

// *******************************************************************

class Torpedo : public EntityCollider {
public:

	Torpedo(bool culling = true);
	~Torpedo();

	float ttl;
	bool ready;

	void update(float elapsed_time);
	void activate();
	void onCollision(EntityCollider* collided_with);
};

// *******************************************************************

class Helix : public EntityMesh {
public:

	Helix();
	~Helix();

};

// *******************************************************************

#endif