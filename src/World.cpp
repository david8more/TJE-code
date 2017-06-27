	
#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "gameentities.h"
#include "explosion.h"
#include "game.h"
#include "shader.h"
#include "soundmanager.h"

#include "states/playstate.h"

#define DEBUG 0
#define IA true

World* World::instance = NULL;
Aircarrier* aircarrier = NULL;
int enemycounter = 0;
Ship* enemyShip = NULL;

World::World()
{
	instance = this;
	root = new Entity();

	sky = NULL;
	playerAir = NULL;
	playerShip = NULL;

	atomic_enabled = false;
	time_to_explode = 15;
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

	playerAir = new Airplane(worldInfo.playerModel, false);
	playerAir->setUid(0);
	playerAir->setName("player");

	// Game mode 
	int mode = Game::instance->difficulty;
	switch (mode)
	{
	case D_SKILLED:
		playerAir->life *= 0.75;
		break;
	case D_INSANE:
		playerAir->life *= 0.5;
		break;
	}

	playerAir->max_life = playerAir->life;
	
	playerAir->model = playerAir->model * aircarrier->model;
	playerAir->model.traslate(0, 17.75, -102.5);
	root->addChild(playerAir);

	playerAir->setDynamic();
	playerAir->createTorpedos();


	// player powerups
	PowerUp * damage = new PowerUp("damage");
	root->addChild(damage);
	powerups.push_back(damage);

	PowerUp * ninja = new PowerUp("ninja");
	root->addChild(ninja);
	powerups.push_back(ninja);

	PowerUp * bomb = new PowerUp("bomb");
	root->addChild(bomb);
	powerups.push_back(bomb);
}

void World::addPlayerConst()
{
	// PLAYER SHIP ************************************************************************************

	playerShip = new Ship(false);
	root->addChild(playerShip);
	ships.push_back(playerShip);

	// initial zone
	aircarrier = new Aircarrier(0, 0);
	root->addChild(aircarrier);

	Aircarrier* ac = new Aircarrier(100, 200);
	root->addChild(ac);

	Aircarrier* ac2 = new Aircarrier(-100, 200);
	root->addChild(ac2);

	Airplane * plane_ac = new Airplane(WILDCAT, false, true, 1);

	plane_ac->model = plane_ac->model * ac->model;
	plane_ac->model.traslate(0, 17.53, -102.5);
	root->addChild(plane_ac);

	Airplane * plane_ac2 = new Airplane(P38, false, true, 1);

	plane_ac2->model = plane_ac2->model * ac2->model;
	plane_ac2->model.traslate(0, 17.6, -102.5);
	root->addChild(plane_ac2);
}

void World::addWorldConst()
{
	Game* game = Game::getInstance();

	// WORLD
	sky = new EntityMesh();
	sky->setName("sky");
	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	
	for (int i = -2; i < 2; i++)
	{
		for (int j = -2; j < 2; j++)
		{
			EntityCollider* sea = new EntityCollider();
			sea->setName("sea");
			sea->set("agua.ASE", "data/textures/agua.tga", "water");
			sea->model.setTranslation(i * 10001, -10, j * 10001);
			root->addChild(sea);
		}
	}

	Clouds* clouds = new Clouds();
	root->addChild(clouds);

	EntityCollider* ground = new EntityCollider();
	ground->set("island.ASE", "data/textures/island_color_luz.tga", "fog");
	ground->model.setIdentity();
	root->addChild(ground);
	ground->setStatic();

	//map_entities.push_back(ground);
}

void World::createReinforcements()
{
	// MORE ENEMIES
	std::stringstream ss;
	for (int i = enemycounter + 1; i < enemycounter + 4; i++)
	{
		Airplane* enemyAir = new Airplane(BOMBER, IA);
		enemyAir->setUid(1000 + i);
		ss.str("");
		ss << "ia_" << i;
		enemyAir->setName(ss.str());
		float x = 500 + rand() % 1500;
		float y = 500 + rand() % 500;
		float z = 500 + rand() % 1500;

		enemyAir->model.setTranslation(x, y, z);
		enemyAir->last_position = enemyAir->getPosition();

		enemyAir->setDynamic();
		airplanes.push_back(enemyAir);
		root->addChild(enemyAir);
	}
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
		IA_enemies = 6;
	else if (game->difficulty == D_SKILLED)
		IA_enemies = 13;
	else if (game->difficulty == D_INSANE)
		IA_enemies = 21;

	for (int i = 1; i < IA_enemies; i++)
	{
		enemycounter++;
		
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

void World::atomic()
{
	for (int i = 0; i < airplanes.size(); i++)
	{
		Airplane * current = (Airplane*)airplanes[i];
		Explosion::createExplosion(current->getPosition(), 150);
		SoundManager::instance->playSound("explosion", false);
		current->destroy();
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

	// asimismo, si matan al barco ->>> LOSE
	if (playerShip->destroyed)
		return true;

	/* vida de los enemigos: si seguimos vivos y ellos mueren ->>> WIN
	*  pero solo acaba si el barco muere!!! 
	*/

	if (enemyShip->destroyed)
	{
		Game::instance->loseWin = true;
		return true;
	}

	// si todo sigue igual:
	return false;
}

void World::reset()
{
	if (playerAir == NULL)
		return;
	
	std::cout << "applying reset to world" << std::endl;

	// enemies
	for (int i = 0; i < airplanes.size(); i++)
		airplanes[i]->destroy();

	for (int i = 0; i < powerups.size(); i++)
		powerups[i]->destroy();

	Entity* enemyShip = Entity::getEntity(Airplane::ENEMY_SHIP);

	if (enemyShip != NULL)
		enemyShip->destroy();

	airplanes.clear();
	ships.clear();

	// player uid is 0
	Entity* player = Entity::getEntity(0);

	if (player != NULL)
		player->destroy();

	Entity* friendlyShip = Entity::getEntity(Airplane::PLAYER_SHIP);

	if (playerShip != NULL)
		playerShip->destroy();
	
	//
	Entity::destroy_entities();
	// 

	// add some constants
	playerShip = new Ship(false);
	root->addChild(playerShip);
	ships.push_back(playerShip);

	atomic_enabled = false;
	Game::instance->end = false;
	Game::instance->start = false;
	Game::instance->loseWin = LOSE;
	Game::instance->score = 0;
}
