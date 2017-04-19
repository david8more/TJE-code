#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "game.h"
#include "shader.h"
#include "playstate.h"

World* World::instance = NULL;

World::World()
{
	instance = this;

	sky = new EntityMesh();
	sea = new EntityMesh();
	ground = new EntityMesh();
	playerAir = new EntityPlayer();
	playerShip = new EntityPlayer();
	root = new Entity();
}

World::~World()
{

}

void World::create() {

	/*	Creacion de todo el mundo excepto el avion del jugador: se añade al elegirlo en el
	*	loading state																	*/
	// *****************************************************************************************
	addWorldConst();
	// *****************************************************************************************
	addPlayerConst();
	// *****************************************************************************************
}

void World::addPlayer() {
	// PLAYER AIRPLANE ************************************************************************************

	switch (worldInfo.playerModel) {
	case SPITFIRE:
		playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		playerAir->life = 250;
		playerAir->cadence = 15.f;
		playerAir->missilesLeft = 2;
		playerAir->damageM60 = 10;
		playerAir->damageMissile = 200;
		break;
	case P38:
		playerAir->set("p38.ASE", "data/textures/p38.tga", "simple");
		playerAir->life = 300;
		playerAir->cadence = 10.f;
		playerAir->missilesLeft = 4;
		playerAir->damageM60 = 5;
		playerAir->damageMissile = 150;
		break;
	case WILDCAT:
		playerAir->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		playerAir->life = 150;
		playerAir->cadence = 30.f;
		playerAir->missilesLeft = 5;
		playerAir->damageM60 = 20;
		playerAir->damageMissile = 300;
		break;
	case BOMBER:
		playerAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "simple");
		playerAir->life = 400;
		playerAir->cadence = 20.f;
		playerAir->missilesLeft = 2;
		playerAir->damageM60 = 30;
		playerAir->damageMissile = 300;
		break;
	}

	// REDUCIR ATRIBUTOS
	if (Game::getInstance()->gameMode == HARD) {
		playerAir->life -= (int)(playerAir->life/(float)4) ;
		playerAir->missilesLeft -= 1;
		playerAir->damageM60 -= 3;
		playerAir->damageMissile -= 100;
	}

	playerAir->model.setScale(3, 3, 3);
	playerAir->model.traslate(0, 500, 500);
	root->addChild(playerAir);
}

void World::addPlayerConst() {

	// PLAYER SHIP ************************************************************************************

	playerShip->set("barco.ASE", "data/textures/barco.tga", "simple");
	playerShip->model.traslateLocal(5500, -2100, 5500);
	playerShip->life = 750;
	root->addChild(playerShip);

	EntityMesh* turretOne = new EntityMesh();;

	turretOne->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretOne->model.setRotation(1.57079633f, Vector3(0.f, 1.f, 0.f));
	turretOne->model = turretOne->model * playerShip->model;
	turretOne->model.traslate(0.f, 5.25f, 66.f);
	root->addChild(turretOne);

	EntityMesh* turretTwo = new EntityMesh();

	turretTwo->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretTwo->model.setRotation(1.57079633f, Vector3(0.f, 1.f, 0.f));
	turretTwo->model = turretTwo->model * playerShip->model;
	turretTwo->model.traslate(0, 8, -63);
	root->addChild(turretTwo);

	EntityMesh* cannonOne = new EntityMesh();

	cannonOne->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonOne->model = cannonOne->model * turretOne->model;
	cannonOne->model.traslate(0.f, 0.01f, 0.f);
	root->addChild(cannonOne);

	EntityMesh* cannonTwo = new EntityMesh();

	cannonTwo->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonTwo->model = cannonTwo->model * turretTwo->model;
	cannonTwo->model.traslate(0.f, 0.01f, 0.f);
	root->addChild(cannonTwo);
}

void World::addWorldConst() {
	
	// WORLD

	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	sky->model.setScale(13, 13, 13);
	sky->model.traslateLocal(0, -200, 0);
	root->addChild(sky);

	sea->set("agua.ASE", "data/textures/agua.tga", "simple");
	sea->model.setScale(3, 3, 3);
	sea->model.traslateLocal(0, -700, 0);
	root->addChild(sea);

	ground->set("island.ASE", "data/textures/island.tga", "simple");
	ground->model.setScale(2, 2, 2);
	ground->model.traslateLocal(0, -1000, 0);
	root->addChild(ground);
}

void World::addEnemies() {
	
	// ENEMIES

	EntityEnemy* enemyShip = new EntityEnemy();

	enemyShip->life = Game::getInstance()->gameMode ? 1500:1000;
	enemyShip->set("barco.ASE", "data/textures/barco.tga", "color");
	enemyShip->model = playerShip->model * enemyShip->model;
	enemyShip->model.traslate(100, 0, 100);
	root->addChild(enemyShip);

	collision_enemies.push_back(enemyShip);

	EntityEnemy* enemyAir = new EntityEnemy();

	enemyAir->life = Game::getInstance()->gameMode ? 225 : 150;
	enemyAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemyAir->model.setScale(3, 3, 3);
	enemyAir->model.traslate(0, 500, 500);
	root->addChild(enemyAir);

	collision_enemies.push_back(enemyAir);

	EntityEnemy* enemy2Air = new EntityEnemy();

	enemy2Air->life = Game::getInstance()->gameMode ? 225 : 150;
	enemy2Air->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemy2Air->model.setScale(3, 3, 3);
	enemy2Air->model.traslate(500, 500, 500);
	root->addChild(enemy2Air);

	collision_enemies.push_back(enemy2Air);
}

bool World::isGameOver() {

	// aqui tratamos de averiguar si se ha llegado al final del juego

	// nuestra vida: si nos matan ->>> LOSE
	if (playerAir->life <= 0) return true;

	/* vida de los enemigos: si seguimos vivos y ellos mueren ->>> WIN
	*  pero solo acaba si el barco muere!!!                         */
	if (collision_enemies[0]->life <= 0) return true;

	// si todos sigue igual:
	return false;
}

void World::reset() {
	
}