
#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "gameentities.h"
#include "game.h"
#include "shader.h"
#include "states/playstate.h"

#define DEBUG 0
#define IA true

World* World::instance = NULL;
std::vector<Entity*> World::airplanes;

Ship* enemyShip = NULL;
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

	playerShip = new Ship(false);
	root->addChild(playerShip);

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

	EntityCollider* ground = new EntityCollider();
	ground->set("island.ASE", "data/textures/island_color_luz.tga", "fog");
	ground->model.setIdentity();
	root->addChild(ground);
	ground->setStatic();

	map_entities.push_back(ground);
}

void World::addEnemies() {
	
	// ENEMIES

	enemyShip = new Ship(IA);
	root->addChild(enemyShip);

	// ***********************************
	std::stringstream ss;

	for (int i = 1; i < 2; i++) {
		Airplane* enemyAir = new Airplane(BOMBER, IA);
		enemyAir->uid = 1000 + i;
		ss.str("");
		ss << "ia_" << i;
		enemyAir->setName(ss.str());
		float x = 200 + rand()%2000;
		float y = 500 + rand()%500;
		float z = 200 + rand()%2000;

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

	
	for (int i = 0; root->children.size(); i++)
	{
		root->children[i]->destroy();
	}
}
