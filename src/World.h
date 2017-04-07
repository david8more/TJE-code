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
	//Meshes
	EntityMesh* sky;
	EntityMesh* sea;
	EntityMesh* ground;
	EntityPlayer * player;
	Entity* root;

	World();
	~World();

	void create();
};

#endif