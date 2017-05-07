#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "gameentities.h"
#include "game.h"
#include "shader.h"
#include "states/playstate.h"

#define DEBUG 0

World* World::instance = NULL;

EntityEnemy* enemyShip = NULL;
EntityEnemy* enemyAir = NULL;
EntityEnemy* enemy2Air = NULL;

World::World()
{
	instance = this;
	root = new Entity();

	sky = NULL;
	sea = NULL;
	playerAir = NULL;
	playerShip = NULL;
}

World::~World()
{

}

void World::create() {

	/*	Creacion de todo el mundo excepto el avion del jugador: se a�ade al elegirlo en el
	*	loading state																	*/
	// *****************************************************************************************
	addWorldConst();
	// *****************************************************************************************
	addPlayerConst();
	// *****************************************************************************************
	addEnemies();
	// *****************************************************************************************
	playerAir = new Airplane();
}

void World::addPlayer() {
	// PLAYER AIRPLANE ************************************************************************************

	switch (worldInfo.playerModel) {
	case SPITFIRE:
		playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		playerAir->life = 350;
		playerAir->cadence = 15.f;
		playerAir->damageM60 = 10;
		playerAir->damageMissile = 250;

		break;
	case P38:
		playerAir->set("p38.ASE", "data/textures/p38.tga", "simple");
		playerAir->life = 400;
		playerAir->cadence = 10.f;
		playerAir->damageM60 = 5;
		playerAir->damageMissile = 150;

		break;
	case WILDCAT:
		playerAir->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		playerAir->life = 250;
		playerAir->cadence = 30.f;
		playerAir->damageM60 = 40;
		playerAir->damageMissile = 100;

		break;
	case BOMBER:
		playerAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "simple");
		playerAir->life = 500;
		playerAir->cadence = 20.f;
		playerAir->damageM60 = 15;
		playerAir->damageMissile = 300;

		break;
	}

	// REDUCIR ATRIBUTOS
	if (Game::getInstance()->gameMode == HARD) {
		playerAir->life -= (int)(playerAir->life/(float)4) ;
		playerAir->damageM60 -= 3;
		playerAir->damageMissile -= 100;
	}

	playerAir->model.setTranslation(0, 500, 500);
	root->addChild(playerAir);
	playerAir->createTorpedos();
}

void World::addPlayerConst() {

	// PLAYER SHIP ************************************************************************************

	playerShip = new Airplane();

	playerShip->set("barco.ASE", "data/textures/barco.tga", "simple");
	//playerShip->model.traslateLocal(5500, -2100, 5500);
	playerShip->life = 750;
	root->addChild(playerShip);

	EntityMesh* turretOne = new EntityMesh();;

	turretOne->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretOne->model.setRotation(1.57079633f, Vector3(0.f, 1.f, 0.f));
	turretOne->model.traslate(0.f, 5.25f, 66.f);
	playerShip->addChild(turretOne);

	EntityMesh* turretTwo = new EntityMesh();

	turretTwo->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretTwo->model.setRotation(1.57079633f, Vector3(0.f, 1.f, 0.f));
	turretTwo->model.traslate(0, 8, -63);
	playerShip->addChild(turretTwo);

	EntityMesh* cannonOne = new EntityMesh();

	cannonOne->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonOne->model.traslate(0.f, 0.01f, 0.f);
	turretOne->addChild(cannonOne);

	EntityMesh* cannonTwo = new EntityMesh();

	cannonTwo->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonTwo->model.traslate(0.f, 0.01f, 0.f);
	turretTwo->addChild(cannonTwo);
}

void World::addWorldConst() {
	
	// WORLD
	sky = new EntityMesh();
	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	
	EntityCollider* sea = new EntityCollider();
	sea->setName("sea");
	sea->set("agua.ASE", "data/textures/agua.tga", "water");
	//sea->setStatic();

	for (int i = -3; i <= 3; i++) {
		for (int j = -3; j <= 3; j++) {
			EntityCollider* ground = new EntityCollider();
			ground->set("island.ASE", "data/textures/island.tga", "fog");
			ground->model.setIdentity();
			ground->model.traslate(i * 14000, 0, j * 14000);
			//ground->model.rotateLocal(0.785398 * i * j, Vector3(0, 1, 0));
			root->addChild(ground);
			//ground->setStatic();
		}
	}


	for (int i = 1; i <= 50; i++) {
		EntityMesh* reload_zone = new EntityMesh(NO_CULLING);
		reload_zone->name = "RELOAD_ZONE";
		reload_zone->set("box.ASE", "data/textures/smoke_alpha_green.tga", "simple");
		reload_zone->model.traslate(0, 200 + i*100, 0);
		root->addChild(reload_zone);
	}
}

void World::addEnemies() {
	
	// ENEMIES

	enemyShip = new EntityEnemy();
	enemyAir = new EntityEnemy();
	enemy2Air = new EntityEnemy();

	enemyShip->setName("enemy_ship");
	enemyShip->life = Game::getInstance()->gameMode ? 1500:1000;
	enemyShip->set("barco.ASE", "data/textures/barco.tga", "color");
	enemyShip->model = playerShip->model * enemyShip->model;
	enemyShip->model.traslate(100, 0, 100);
	root->addChild(enemyShip);

	enemyShip->setStatic();

	enemyAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemyAir->model.setTranslation(0, 500, 200);
	root->addChild(enemyAir);

	enemyAir->setStatic();

	enemy2Air->life = Game::getInstance()->gameMode ? 225 : 150;
	enemy2Air->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemy2Air->model.setTranslation(500, 500, 200);
	root->addChild(enemy2Air);

	enemyAir->setStatic();

	if (!DEBUG) return;

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 20; j++) {
			EntityMesh* p = new EntityMesh();
			p->name = "p";
			p->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
			float x = rand() % 800;
			float y = rand() % 800;
			float z = rand() % 800;
			p->model.traslate(x, y, z);
			root->addChild(p);
		}
	}
}

void World::setGameMode() {
	enemyAir->life = Game::getInstance()->gameMode ? 225 : 150;
	enemy2Air->life = Game::getInstance()->gameMode ? 225 : 150;
	enemyShip->life = Game::getInstance()->gameMode ? 1500 : 1000;
}

bool World::isGameOver() {

	// aqui tratamos de averiguar si se ha llegado al final del juego

	// nuestra vida: si nos matan ->>> LOSE
	if (playerAir->life <= 0)
		return true;

	/* vida de los enemigos: si seguimos vivos y ellos mueren ->>> WIN
	*  pero solo acaba si el barco muere!!! 
	*/

	if (enemyShip->life <= 0)
		return true;

	// si todo sigue igual:
	return false;
}

void World::reset() {
	
}