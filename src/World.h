#ifndef WORLD_H
#define WORLD_H

#include "framework.h"
#include "utils.h"
#include "entity.h"
#include "gameentities.h"

class Entity;
class EntityMesh;
class EntityCollider;
class Airplane;
class EntityEnemy;
class Torpedo;

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

	Entity* root;

	//world constants
	EntityMesh* sky;
	EntityCollider* sea;

	// fighters
	Airplane* playerAir;
	Airplane* playerShip;

	void create();
	void addPlayer();
	void addPlayerConst();
	void addEnemies();
	void setGameMode();
	void addWorldConst();

	void reset();
	bool isGameOver();
};

#endif