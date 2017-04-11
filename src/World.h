#ifndef WORLD_H
#define WORLD_H

#include "framework.h"
#include "utils.h"

class Entity;
class EntityMesh;
class EntityPlayer;

class World
{
public:

	static World* instance;

	// SINGLETON
	static World* getInstance() {
		return instance;
	}


	//Meshes
	EntityMesh* sky;
	EntityMesh* sea;
	EntityMesh* ground;

	// fighters
	EntityPlayer* playerAir;
	EntityMesh* helix;
	EntityPlayer* playerShip;
	Entity* root;

	World();
	~World();

	void create();
};

#endif