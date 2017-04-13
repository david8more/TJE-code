#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "game.h"
#include "shader.h"

World* World::instance = NULL;

World::World()
{
	instance = this;

	sky = new EntityMesh();
	sea = new EntityMesh();
	ground = new EntityMesh();
	playerAir = new EntityPlayer();
	playerShip = new EntityPlayer();
	enemyShip = new EntityMesh();
	root = new Entity();
}

World::~World()
{

}

void World::create() {

	// PLAYER AIRPLANE ************************************************************************************

	switch (worldInfo.playerModel) {
	case 0:
		playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		break;
	case 1:
		playerAir->set("p38.ASE", "data/textures/p38.tga", "simple");
		break;
	case 2:
		playerAir->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		break;
	}

	playerAir->model.setScale(3, 3, 3);
	playerAir->model.traslate(0, 500, 500);
	root->addChild(playerAir);

	// PLAYER SHIP ************************************************************************************

	playerShip->set("barco.ASE", "data/textures/barco.tga", "simple");
	playerAir->model.setScale(1, 1, 1);
	playerShip->model.traslateLocal(5500, -2100, 5500);
	root->addChild(playerShip);

	EntityMesh* turretOne = new EntityMesh();;

	turretOne->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretOne->model.setRotation(1.57079633, Vector3(0, 1, 0));
	turretOne->model = turretOne->model * playerShip->model;
	turretOne->model.traslate(0, 5.25, 66);
	root->addChild(turretOne);

	EntityMesh* turretTwo = new EntityMesh();

	turretTwo->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretTwo->model.setRotation(1.57079633, Vector3(0, 1, 0));
	turretTwo->model = turretTwo->model * playerShip->model;
	turretTwo->model.traslate(0, 8, -63);
	root->addChild(turretTwo);

	EntityMesh* cannonOne = new EntityMesh();

	cannonOne->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonOne->model = cannonOne->model * turretOne->model;
	cannonOne->model.traslate(0, 0.01, 0);
	root->addChild(cannonOne);

	EntityMesh* cannonTwo = new EntityMesh();

	cannonTwo->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonTwo->model = cannonTwo->model * turretTwo->model;
	cannonTwo->model.traslate(0, 0.01, 0);
	root->addChild(cannonTwo);

	// *****************************************************************************************

	// WORLD

	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	sky->model.setScale(13, 13, 13);
	sky->model.traslateLocal(0, -200, 0);
	root->addChild(sky);

	sea->set("agua.ASE", "data/textures/agua.tga", "simple");
	sea->model.setScale(3, 3, 3);
	sea->model.traslateLocal(0, -700, 0);
	root->addChild(sea);
	
	ground->set("island.ASE", "data/textures/island_color.tga", "simple");
	ground->model.setScale(2, 2, 2);
	ground->model.traslateLocal(0, -1000, 0);
	root->addChild(ground);

	// ENEMIES

	enemyShip->set("barco.ASE", "data/textures/barco.tga", "color");
	enemyShip->model = playerShip->model * enemyShip->model;
	enemyShip->model.traslate(100, 0, 100);
	root->addChild(enemyShip);

	collision_enemies.push_back(enemyShip);
}