#ifndef GAMEENTITIES_H
#define GAMEENTITIES_H

#include "entity.h"

class IAController;
class Missile;

class Airplane : public EntityCollider {
public:

	Airplane(int model, bool IA_PLANE, bool culling = true);
	~Airplane();

	IAController* controller;

	int planeModel;
	std::string state;

	bool engine;
	void engineOnOff();

	double wheels_rotation;

	int torpedosLeft;
	float speed;
	float cadence;
	float damageM60;

	// m60 attributes
	float timer;
	bool overused;
	int shootingtime;
	float last_shoot;

	Torpedo* torpedos[2];

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void shoot();
	void createTorpedos();
	void torpedoShoot();
	void onCollision(EntityCollider* collided_with);
	void unboundController();

	enum {
		PLAYER_SHIP = 1,
		ENEMY_SHIP = 2
	};
};

// *******************************************************************

class Ship : public EntityCollider {

public:
	Ship(bool ia);
	~Ship();

	std::string state;

	float damage;
	float last_shoot;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void shoot();
	void onCollision(EntityCollider* collided_with);

	std::vector<Missile*> missiles;

};

// *******************************************************************

class Torpedo : public EntityCollider {
public:

	Torpedo(bool culling = true);
	~Torpedo();

	float ttl;
	float max_ttl;
	bool ready;

	void update(float elapsed_time);
	void activate();
	void onCollision(EntityCollider* collided_with);
};

// *******************************************************************

class Missile : public EntityCollider {
public:

	Missile(bool culling = true);
	~Missile();

	float ttl;
	float max_ttl;

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

class Clouds : public EntityMesh
{
public:
	typedef struct {
		Vector3 pos;
		float size;
		float distance;
	}sCloudInfo;

	Clouds();

	void render(Camera* cam);

	std::vector<sCloudInfo> clouds;
};

// *******************************************************************

#endif