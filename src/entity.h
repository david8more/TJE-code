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
	bool visible;
	string name;
	Entity* parent;
	vector<Entity*> children;

	Mesh* mesh;
	Texture* texture;
	Shader *shader;

	virtual void render(Camera * camera);
	virtual void update(float elapsed_time);
	void removeChild(unsigned int uid);
	void addChild(Entity* entity);
	Matrix44 getGlobalMatrix();
	Vector3 getPosition();

	string& getName() {
		return name;
	}

	void setName( string name ) {
		this->name = name;
	}
	
};

// *******************************************************************
// adding mesh and texture to get the visual representation

class EntityMesh: public Entity {
public:

	EntityMesh();
	~EntityMesh();

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);

};

// *******************************************************************
class EntityPlayer : public Entity {
public:

	EntityPlayer();
	~EntityPlayer();

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

	int life;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);
	//void m60Shoot();
	//void missileShoot();
};

#endif