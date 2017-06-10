
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

Ship* enemyShip = NULL;
EntityCollider* init_zone = NULL;

World::World()
{
	instance = this;
	root = new Entity();

	sky = NULL;
	playerAir = NULL;
	playerShip = NULL;
}

World::~World()
{

}

void World::create()
{
	/*
		Creacion de las constantes del mundo:
		Jugador y enemigos se crean cuando se sabe el nivel escogido
	*/

	// *****************************************************************************************
	addWorldConst();
	// *****************************************************************************************
	addPlayerConst();
	// *****************************************************************************************
}

void World::addPlayer()
{
	// PLAYER AIRPLANE ************************************************************************************

	playerAir = new Airplane(worldInfo.playerModel, NULL);
	playerAir->setUid(0);
	playerAir->setName("player");

	// Game mode 
	int mode = Game::instance->difficulty;
	switch (mode)
	{
	case D_BABY:
		playerAir->life += 25;
		playerAir->damageM60 += 10.0;
		break;
	case D_SKILLED:
		playerAir->life *= 0.75;
		playerAir->damageM60 -= 5.0;
		break;
	case D_INSANE:
		playerAir->life *= 0.5;
		playerAir->damageM60 *= 0.5;
		break;
	}

	playerAir->max_life = playerAir->life;
	
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
	ships.push_back(playerShip);

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
	ships.push_back(enemyShip);

	// ***********************************
	Game* game = Game::instance;
	std::stringstream ss;
	int IA_enemies;

	if (game->difficulty == D_BABY)
		IA_enemies = 3;
	if (game->difficulty == D_SKILLED)
		IA_enemies = 8;
	if (game->difficulty == D_INSANE)
		IA_enemies = 20;

	for (int i = 1; i < IA_enemies; i++) {
		//int type = rand() % 4;
		Airplane* enemyAir = new Airplane(BOMBER, IA);
		enemyAir->setUid(1000 + i);
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

	for (int i = 0; i < EntityCollider::dynamic_colliders.size(); i++)
	{
		EntityCollider * current_collider = EntityCollider::dynamic_colliders[i];
		Mesh::Get(current_collider->mesh.c_str())->setCollisionModel();
	}
}

void World::setGameMode()
{
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

void World::reset()
{
	std::cout << "applying reset to world" << std::endl;

	// enemies
	for (int i = 0; i < airplanes.size(); i++)
		airplanes[i]->destroy();

	Entity* enemyShip = Entity::getEntity(Airplane::ENEMY_SHIP);

	if (enemyShip != NULL)
		enemyShip->destroy();

	// player
	playerAir->destroy();
	if (playerShip != NULL)
		playerShip->destroy();

	Entity::destroy_entities();

	playerShip = new Ship(false);
	root->addChild(playerShip);
	ships.push_back(playerShip);

	Game::instance->end = false;
}
