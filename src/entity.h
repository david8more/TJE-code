#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "utils.h"
#include <iostream>

#define NO_CULLING false
#define CULLING true

using namespace std;

class Mesh;
class Texture;
class Shader;
class Torpedo;

class Camera;

class Entity {
public:	
	Entity();
	~Entity();

	Matrix44 model;
	unsigned int uid;
	string name;
	Entity* parent;
	vector<Entity*> children;

	virtual void render(Camera * camera);
	virtual void update(float elapsed_time);
	void removeChild(Entity* entity);
	void addChild(Entity* entity);
	Matrix44 getGlobalMatrix();
	Vector3 getPosition();

	void destroy();
	static void destroy_entities();
	static std::vector<Entity*> destroy_pending;
	
};

// *******************************************************************
// adding mesh and texture to get the visual representation

class EntityMesh: public Entity {
public:

	EntityMesh(bool culling = true);
	~EntityMesh();

	std::string mesh;
	std::string texture;
	Shader* shader;
	bool culling;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);

};

// *******************************************************************
class EntityPlayer : public EntityMesh {
public:

	EntityPlayer(bool culling = true);
	~EntityPlayer();

	int life;
	int torpedosLeft;
	float cadence;
	float damageM60;
	float damageMissile;

	Torpedo* torpedos[2];

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void m60Shoot();
	void createTorpedos();
	void torpedoShoot();
};

// *******************************************************************
class EntityEnemy : public EntityMesh {
public:

	EntityEnemy(bool culling = true);
	~EntityEnemy();

	int life;
	CollisionModel3D* collision_model;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	//void m60Shoot();
	//void missileShoot();
};

// *******************************************************************
class Torpedo : public EntityMesh {
public:

	Torpedo(bool culling = true);
	~Torpedo();

	float ttl;
	static unsigned int last_tid;
	unsigned int tid;
	bool ready;

	void update(float elapsed_time);
	void activate();
};

#endif