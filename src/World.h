#ifndef WORLD_H
#define WORLD_H

#include "framework.h"
#include "utils.h"
#include "entity.h"
#include "gameentities.h"
#include "iacontroller.h"

#define WIN 1
#define LOSE 0

class Entity;
class EntityMesh;
class EntityCollider;
class Airplane;
class Aircarrier;
class EntityEnemy;
class Torpedo;
class PowerUp;

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

	bool atomic_enabled;
	float time_to_explode;

	Entity* root;

	//world constants
	EntityMesh* sky;

	std::vector<Entity*> map_entities;
	std::vector<Entity*> powerups;
	std::vector<Entity*> airplanes;
	std::vector<Entity*> ships;

	// fighters
	Airplane* playerAir;
	Ship* playerShip;

	void create();
	void addPlayer();
	void addPlayerConst();
	void addEnemies();
	void atomic();
	void setGameMode();
	void addWorldConst();

	void createReinforcements();

	void reset();
	bool isGameOver();
};

#endif