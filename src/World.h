#ifndef WORLD_H
#define WORLD_H

#include "framework.h"
#include "utils.h"

class Entity;
class EntityMesh;
class EntityPlayer;
class EntityEnemy;

class World
{

private:

	World();
	~World();

public:
	
	static World* instance;

	// SINGLETON
	static World* getInstance() {
		if (instance == NULL) instance = new World();
		return instance;
	}

	typedef struct {
		int playerModel;
		int rivalModel;
	}sWorldInfo;

	sWorldInfo worldInfo;

	// colisionables??
	std::vector<EntityEnemy*> collision_enemies;

	Entity* root;

	//world constants
	EntityMesh* sky;
	EntityMesh* sea;
	EntityMesh* ground;

	// fighters
	EntityPlayer* playerAir;
	EntityPlayer* playerShip;

	void create();
	void addPlayer();
	void addPlayerConst();
	void addEnemies();
	void addWorldConst();

	void reset();
	bool isGameOver();
};

#endif