#ifndef GAMEENTITIES_H
#define GAMEENTITIES_H

#include "entity.h"

class Airplane : public EntityCollider {
public:

	Airplane(int model, bool culling = true);
	~Airplane();

	Vector3 last_position;

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
	float cadenceCounter;

	Torpedo* torpedos[2];

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void m60Shoot();
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