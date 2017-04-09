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
	playerAir = new EntityPlayer();
	playerShip = new EntityPlayer();
	helix = new EntityMesh();
	root = new Entity();
}

World::~World()
{

}

void World::create() {

	playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
	playerAir->model.setScale(3, 3, 3);
	playerAir->model.traslate(0, 550, 500);
	//root->addChild(playerAir);

	playerShip->set("barco.ASE", "data/textures/barco.tga", "simple");
	playerShip->model.traslate(0, 500, 500);
	//root->addChild(playerShip);

	EntityMesh* turretOne = new EntityMesh();;

	turretOne->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretOne->model.setRotation(1.57079633, Vector3(0, 1, 0));
	turretOne->model = turretOne->model * playerShip->model;
	turretOne->model.traslate(0, 5.25, 66);
	//root->addChild(turretOne);

	EntityMesh* turretTwo = new EntityMesh();

	turretTwo->set("barco_turret.ASE", "data/textures/barco_turret.tga", "simple");
	turretTwo->model.setRotation(1.57079633, Vector3(0, 1, 0));
	turretTwo->model = turretTwo->model * playerShip->model;
	turretTwo->model.traslate(0, 8, -63);
	//root->addChild(turretTwo);

	EntityMesh* cannonOne = new EntityMesh();

	cannonOne->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonOne->model = cannonOne->model * turretOne->model;
	cannonOne->model.traslate(0, 0.01, 0);
	//root->addChild(cannonOne);

	EntityMesh* cannonTwo = new EntityMesh();

	cannonTwo->set("barco_cannons.ASE", "data/textures/barco_turret.tga", "simple");
	cannonTwo->model = cannonTwo->model * turretTwo->model;
	cannonTwo->model.traslate(0, 0.01, 0);
	//root->addChild(cannonTwo);

	EntityMesh* aux = new EntityMesh();

	aux->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
	aux->model.traslate(0, 500, 500);
	root->addChild(aux);

	helix->set("helice.ASE", "data/textures/helice.tga", "simple");
	helix->model.setScale(0.75, 0.75, 0.75);
	helix->model = helix->model * aux->model;
	helix->model.traslate(0, 0, 3.5);
	root->addChild(helix);

	/*sky->set("cielo.ASE", "data/textures/cielo.tga", "color");
	sky->model.setTranslation(1, 1, 1);
	sky->model.scale(10, 10, 10);
	root->addChild(sky);

	sea->set("agua.ASE", "data/textures/agua.tga", "color");
	sea->model.setTranslation(1, 25, 1);
	sea->model.scale(10, 10, 10);
	root->addChild(sea);

	ground->set("island.ASE", "data/textures/island_color.tga", "color");
	ground->model.setTranslation(1, 200, 1);
	ground->model.scale(2, 2, 2);
	root->addChild(ground);*/
}