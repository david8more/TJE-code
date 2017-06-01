
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
std::vector<Entity*> World::airplanes;

EntityCollider* enemyShip = NULL;
EntityCollider* init_zone = NULL;

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

void World::create()
{
	/*	Creacion de todo el mundo excepto el avion del jugador: se añade al elegirlo en el
	*	loading state																	*/
	// *****************************************************************************************
	addWorldConst();
	// *****************************************************************************************
	addPlayerConst();
	// *****************************************************************************************
	addEnemies();
	// *****************************************************************************************
}

void World::addPlayer()
{
	// PLAYER AIRPLANE ************************************************************************************

	playerAir = new Airplane(worldInfo.playerModel, NULL);
	playerAir->setName("player");

	// REDUCIR ATRIBUTOS
	if (Game::getInstance()->gameMode == HARD)
	{
		playerAir->life -= (int)(playerAir->life / 4.f) ;
		playerAir->damageM60 -= 5.0;
	}
	
	playerAir->model = playerAir->model * init_zone->model;
	playerAir->model.traslate(0, 17.75, -102.5);
	root->addChild(playerAir);

	playerAir->setDynamic();
	playerAir->createTorpedos();
}

void World::addPlayerConst()
{
	// PLAYER SHIP ************************************************************************************

	playerShip = new EntityCollider();
	playerShip->setName("ship");
	playerShip->set("barco.ASE", "data/textures/barco.tga", "simple");
	playerShip->model.setTranslation(1600, -10, 1600);
	playerShip->life = 750;

	playerShip->setStatic();
	if(Game::getInstance()->ffire_on)
		playerShip->setDynamic();

	root->addChild(playerShip);

	EntityMesh* turretOne = new EntityMesh();

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

	// initial zone
	init_zone = new EntityCollider();
	init_zone->set("aircarrier.ASE", "data/textures/aircarrier_metal.tga", "simple");
	init_zone->model.setTranslation(2000, -10, -2000);
	init_zone->setStatic();
	root->addChild(init_zone);
}

void World::addWorldConst()
{
	Game* game = Game::getInstance();

	// WORLD
	sky = new EntityMesh();
	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	
	EntityCollider* sea = new EntityCollider();
	sea->setName("sea");
	sea->set("agua.ASE", "data/textures/agua.tga", "water");

	Clouds* clouds = new Clouds();
	root->addChild(clouds);

	if (0)
	{
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				EntityCollider* ground = new EntityCollider();
				ground->set("island.ASE", "data/textures/island_color_luz.tga", "fog");
				ground->model.setIdentity();
				ground->model.traslate(i * 10000, 0, j * 10000);
				ground->model.rotateLocal(0.785398 * i * j, Vector3(0, 1, 0));
				root->addChild(ground);
				ground->setStatic();

				map_entities.push_back(ground);
			}
		}
	}
	else
	{
		EntityCollider* ground = new EntityCollider();
		ground->set("island.ASE", "data/textures/island_color_luz.tga", "fog");
		ground->model.setIdentity();
		root->addChild(ground);
		ground->setStatic();

		map_entities.push_back(ground);
	}
}

void World::addEnemies() {
	
	// ENEMIES

	enemyShip = new EntityCollider();

	enemyShip->setName("ship");
	enemyShip->setLife(1000);
	enemyShip->set("barco.ASE", "data/textures/barco.tga", "color");
	enemyShip->model = playerShip->model * enemyShip->model;
	enemyShip->model.traslate(100, 0, 100);
	root->addChild(enemyShip);

	enemyShip->setStatic();
	enemyShip->setDynamic();

	// ***********************************
	std::stringstream ss;

	for (int i = 1; i < 5; i++) {
		Airplane* enemyAir = new Airplane(BOMBER, new IAController());
		enemyAir->uid = 1000 + i;
		ss.str("");
		ss << "ia_" << i;
		enemyAir->setName(ss.str());
		float x = 200 + rand()%1000;
		float y = 500 + rand()%200;
		float z = 200 + rand()%1000;

		enemyAir->model.setTranslation(x, y, z);
		enemyAir->last_position = enemyAir->getPosition();

		enemyAir->setDynamic();
		airplanes.push_back(enemyAir);
		root->addChild(enemyAir);
	}
}

void World::removeAirplaneFromMinimap(Entity* plane)
{
	// minimap
	auto it = std::find(airplanes.begin(),airplanes.end(), plane);

	if (it != airplanes.end())
		airplanes.erase(it);
}

void World::setGameMode()
{

	Game* game = Game::getInstance();

	// TODO
}

bool World::isGameOver()
{

	// aqui tratamos de averiguar si se ha llegado al final del juego

	// nuestra vida: si nos matan ->>> LOSE
	if (playerAir->life <= 0)
		return true;

	/* vida de los enemigos: si seguimos vivos y ellos mueren ->>> WIN
	*  pero solo acaba si el barco muere!!! 
	*/

	if (enemyShip->destroyed)
		return true;

	// si todo sigue igual:
	return false;
}

void World::reset() {
	
}