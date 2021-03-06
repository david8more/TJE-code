#ifndef GAMEENTITIES_H
#define GAMEENTITIES_H

#include "entity.h"
#include "mesh.h"

class Aircarrier;
class IAController;
class Missile;

class Airplane : public EntityCollider {
public:

	Airplane(int model, bool IA_PLANE, bool culling = true, int decoration = 0);
	~Airplane();

	IAController* controller;

	int planeModel;
	int visibility;
	int decoration;

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

	// 0 mesh
	// 1 texture
	std::string originals[2];

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void shoot();
	void rear_shoot();
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
	bool cannonReady;

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

class Aircarrier : public EntityCollider {
public:

	std::string texture2;
	int materialTriangle;

	Aircarrier(int i, int j);
	~Aircarrier();

	void set(const char * mesh, const char * texture1, const char* texture2, const char * shader);
	void render(Camera * camera);
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

	bool hasCollided;
	float ttl;
	float max_ttl;
	Mesh rastro;

	void render(Camera* camera);
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

class PowerUp : public EntityCollider {

public:
	PowerUp(const std::string&  type);
	~PowerUp();

	static bool damageTaken;
	static bool ninjaTaken;

	void set(const char * mesh, const char * texture, const char * shader);
	void update(float elapsed_time);
	void execute();
	static void reset();
	void onCollision(EntityCollider* collided_with);

	enum {
		DAMAGE = 501,
		NINJA = 502,
		BOMB = 503
	};

};
// *******************************************************************

#endif