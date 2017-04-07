#include "World.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "game.h"
#include "shader.h"

World::World()
{
	sky = new EntityMesh();
	sea = new EntityMesh();
	ground = new EntityMesh();
	player = new EntityPlayer();
	root = new Entity();
}

World::~World()
{
	delete(player);
}

void World::create() {
	player->set("spitfire.ASE", "data/textures/spitfire.tga", "texture");
	player->model.setTranslation(0, 1200, 0);
	player->model.scale(1, 1, 1);
	root->addChild(player);

	sky->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	sky->model.setTranslation(1, 1, 1);
	sky->model.scale(10, 10, 10);
	root->addChild(sky);

	//sea->set("agua.ASE", "data/textures/agua.tga", "simple");
	//sea->model.setTranslation(1, 1, 1);
	//root->addChild(sea);

	ground->set("island.ASE", "data/textures/island_color.tga", "texture");
	ground->model.setTranslation(1, 200, 1);
	ground->model.scale(2, 2, 2);
	root->addChild(ground);
}