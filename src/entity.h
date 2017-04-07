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

	Mesh* mesh;
	Texture* texture;
	Shader *shader;

	virtual void render();
	virtual void update(float elapsed_time);
	void removeChild(Entity* entity);
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
	void render();
	void update(float elapsed_time);

};

// *******************************************************************
// adding mesh and texture to get the visual representation

class EntityPlayer : public Entity {
public:

	EntityPlayer();
	~EntityPlayer();

	void set(const char * mesh, const char * texture, const char * shader);
	void render();
	void update(float elapsed_time);

};

#endif