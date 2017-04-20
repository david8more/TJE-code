#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "utils.h"
#include <iostream>
using namespace std;

class Mesh;
class Texture;
class Shader;

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
	
};

// *******************************************************************
// adding mesh and texture to get the visual representation

class EntityMesh: public Entity {
public:

	EntityMesh();
	~EntityMesh();

	Mesh* mesh;
	Texture* texture;
	Shader *shader;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);

};

// *******************************************************************
class EntityPlayer : public Entity {
public:

	EntityPlayer();
	~EntityPlayer();

	Mesh* mesh;
	Texture* texture;
	Shader *shader;

	int life;
	int missilesLeft;
	float cadence;
	float damageM60;
	float damageMissile;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	void m60Shoot();
	void missileShoot();
};

// *******************************************************************
class EntityEnemy : public Entity {
public:

	EntityEnemy();
	~EntityEnemy();

	Mesh* mesh;
	Texture* texture;
	Shader *shader;

	int life;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	//void m60Shoot();
	//void missileShoot();
};

#endif