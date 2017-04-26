#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "game.h"
#include "shader.h"
#include "states/playstate.h"

World* World::instance = NULL;

EntityEnemy* enemyShip = NULL;
EntityEnemy* enemyAir = NULL;
EntityEnemy* enemy2Air = NULL;

World::World()
{
	instance = this;
	root = new Entity();

	// poner aqui en NULL
	sky = NULL;
	sea = NULL;
	playerAir = NULL;
	playerShip = NULL;
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
	addEnemies();
	// *****************************************************************************************
	playerAir = new EntityPlayer();
}

void World::addPlayer() {
	// PLAYER AIRPLANE ************************************************************************************
	
	Torpedo * t2 = new Torpedo();
	Torpedo * t1 = new Torpedo();

	switch (worldInfo.playerModel) {
	case SPITFIRE:
		playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		playerAir->life = 350;
		playerAir->cadence = 15.f;
		playerAir->torpedosLeft = 2;
		playerAir->damageM60 = 10;
		playerAir->damageMissile = 250;

		t1->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t1->model.traslate(0.75f, -0.75f, -0.5f);
		t2->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t2->model.traslate(-0.75f, -0.75f, -0.5f);

		break;
	case P38:
		playerAir->set("p38.ASE", "data/textures/p38.tga", "simple");
		playerAir->life = 400;
		playerAir->cadence = 10.f;
		playerAir->torpedosLeft = 2;
		playerAir->damageM60 = 5;
		playerAir->damageMissile = 150;

		t1->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t1->model.traslate(1.25f, -0.3f, -0.5f);
		t2->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t2->model.traslate(-1.25f, -0.3f, -0.5f);

		break;
	case WILDCAT:
		playerAir->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		playerAir->life = 250;
		playerAir->cadence = 30.f;
		playerAir->torpedosLeft = 2;
		playerAir->damageM60 = 40;
		playerAir->damageMissile = 100;

		t1->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t1->model.traslate(0.5f, -0.75f, -0.5f);
		t2->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t2->model.traslate(-0.5f, -0.75f, -0.5f);

		break;
	case BOMBER:
		playerAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "simple");
		playerAir->life = 500;
		playerAir->cadence = 20.f;
		playerAir->torpedosLeft = 2;
		playerAir->damageM60 = 15;
		playerAir->damageMissile = 300;

		t1->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t1->model.traslate(1.5f, -1.75f, 0.25f);
		t2->model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));
		t2->model.traslate(-1.5f, -1.75f, 0.25f);

		break;
	}

	// REDUCIR ATRIBUTOS
	if (Game::getInstance()->gameMode == HARD) {
		playerAir->life -= (int)(playerAir->life/(float)4) ;
		playerAir->damageM60 -= 3;
		playerAir->damageMissile -= 100;
	}

	playerAir->torpedos.push_back(t1);
	playerAir->torpedos.push_back(t2);

	playerAir->addChild(t1);
	playerAir->addChild(t2);

	playerAir->model.setScale(3, 3, 3);
	playerAir->model.traslate(0, 500, 500);
	root->addChild(playerAir);

}

void World::addPlayerConst() {

	// PLAYER SHIP ************************************************************************************

	playerShip = new EntityPlayer();

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
	
	for (int i = -3; i <= 3; i++) {
		for (int j = -3; j <= 3; j++) {
			EntityMesh* ground = new EntityMesh();
			ground->set("island.ASE", "data/textures/island.tga", "simple");
			ground->model.setIdentity();
			ground->model.traslate(i * 14000, 0, j * 14000);
			//ground->model.rotateLocal(0.785398 * i * j, Vector3(0, 1, 0));
			root->addChild(ground);
		}
	}

	for (int i = -5; i <= 5; i++) {
		for (int j = -5; j <= 5; j++) {
			EntityMesh* sea = new EntityMesh();
			sea->set("agua.ASE", "data/textures/agua.tga", "color");
			sea->model.setIdentity();
			sea->model.traslate(i * 10000, 0, j * 10000);
			//root->addChild(sea);
		}
	}
}

void World::addEnemies() {
	
	// ENEMIES

	enemyShip = new EntityEnemy();
	enemyAir = new EntityEnemy();
	enemy2Air = new EntityEnemy();

	enemyShip->life = Game::getInstance()->gameMode ? 1500:1000;
	enemyShip->set("barco.ASE", "data/textures/barco.tga", "color");
	enemyShip->model = playerShip->model * enemyShip->model;
	enemyShip->model.traslate(100, 0, 100);
	root->addChild(enemyShip);

	collision_enemies.push_back(enemyShip);

	enemyAir->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemyAir->model.setTranslation(0, 500, 200);
	root->addChild(enemyAir);

	collision_enemies.push_back(enemyAir);

	enemy2Air->life = Game::getInstance()->gameMode ? 225 : 150;
	enemy2Air->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "color");
	enemy2Air->model.setTranslation(500, 500, 200);
	root->addChild(enemy2Air);

	collision_enemies.push_back(enemy2Air);

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
	if (playerAir->life <= 0) return true;

	/* vida de los enemigos: si seguimos vivos y ellos mueren ->>> WIN
	*  pero solo acaba si el barco muere!!!                         */
	if (collision_enemies[0]->life <= 0) return true;

	// si todos sigue igual:
	return false;
}

void World::reset() {
	
}