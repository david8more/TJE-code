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

	// colisionables??
	std::vector<EntityMesh*> collision_enemies;

	//Meshes
	EntityMesh* sky;
	EntityMesh* sea;
	EntityMesh* ground;

	// fighters
	typedef struct {
		int playerModel;
		int rivalModel;
	}sWorldInfo;

	sWorldInfo worldInfo;

	EntityPlayer* playerAir;
	EntityPlayer* playerShip;
	EntityMesh* enemyShip;
	Entity* root;

	World();
	~World();

	void create();
};

#endif