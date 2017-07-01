#include "states/state.h"
#include "states/playstate.h"
#include "states\endingstate.h"
#include "gameentities.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "shader.h"
#include "bulletmanager.h"
#include "playercontroller.h"
#include "soundmanager.h"
#include "world.h"
#include "mesh.h"
#include "explosion.h"
#include "effects.h"
#include <algorithm>
#include <cassert>

// *************************************************************************
// AIRPLANE 
// *************************************************************************

Airplane::Airplane(int model, bool ia, bool culling, int decoration)
	: decoration(decoration), planeModel(model)
{
	EntityCollider* wh_right = new EntityCollider();
	wh_right->setName("right wheel");
	wh_right->set("spitfire_wheel_right.ASE", "data/textures/spitfire.tga", "plane");
	
	EntityCollider* wh_left = new EntityCollider();
	wh_left->setName("left wheel");
	wh_left->set("spitfire_wheel_left.ASE", "data/textures/spitfire.tga", "plane");

	Helix* helix = new Helix();
	helix->setName("helix");
	Helix* helix2 = new Helix();
	helix2->setName("helix_2");

	if (model == SPITFIRE)
	{
		set("spitfire.ASE", "data/textures/spitfire.tga", "spitfire");
		helix->model.setTranslation(0.f, 0.f, 2.1f);

		wh_right->model.setTranslation(-0.82, -0.58, 0.16);
		wh_right->model.rotateLocal(90.0 * DEG2RAD, Vector3(0, 0, -1));
		addChild(wh_right);

		wh_left->model.setTranslation(0.82, -0.58, 0.16);
		wh_left->model.rotateLocal(90.0 * DEG2RAD, Vector3(0, 0, 1));
		addChild(wh_left);

		setLife(100);
		cadence = 70.0 + random() * 10;;
		damageM60 = 17.5;
		speed = 120.0;
	}

	else if (model == P38)
	{
		set("p38.ASE", "data/textures/p38.tga", "p38");
		helix->model.setTranslation(2.44f, 0.f, 2.85f);

		helix2->model.setTranslation(-2.44f, 0.f, 2.85f);
		helix2->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
		addChild(helix2);

		setLife(125);
		cadence = 80.0 + random() * 10;
		damageM60 = 12.5;
		speed = 110.0;
	}

	else if (model == WILDCAT)
	{
		set("wildcat.ASE", "data/textures/wildcat.tga", "wildcat");
		helix->model.setTranslation(0.f, 0.f, 3.1f);

		setLife(150);
		cadence = 60.f + random() * 10;;
		damageM60 = 25.0;
		speed = 90.0;
	}

	else if (model == BOMBER)
	{
		set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "plane");
		helix->model.setTranslation(2.65f, -0.88f, 4.65f);

		helix2->model.setTranslation(-2.65f, -0.88f, 4.65f);
		helix2->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
		addChild(helix2);

		setLife(175);
		cadence = 50.0 + random() * 10;;
		damageM60 = 20.0;
		speed = 100.0;
	}
	else
	{
		exit(0);
	}

	helix->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
	addChild(helix);

	// plane properties
	engine = false;
	wheels_rotation = 0;
	last_shoot = 0;
	visibility = 1000;
	is_ia = ia;

	//m60 init
	timer = shootingtime = 0;
	overused = false;

	// controller

	controller = NULL;

	if (ia)
	{
		controller = new IAController();
		controller->setPlayer(this);
	}
}

Airplane::~Airplane()
{
	//std::cout << "deleting airplane" << std::endl;
	World* world = World::getInstance();

	auto it = std::find(world->airplanes.begin(), world->airplanes.end(), this);
	if (it != world->airplanes.end())
	{
		std::cout << "erasing that old airplane..." << std::endl;
		world->airplanes.erase(it);
	}
}

void Airplane::engineOnOff()
{
	engine = !engine;
}

//meshfile sin path, texturefile con path
void Airplane::set(const char * meshf, const char * texturef, const char * shaderf)
{
	mesh = meshf;
	texture = texturef;

	originals[0] = meshf;
	originals[1] = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void Airplane::render(Camera * camera)
{
	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 center = Mesh::Get(mesh.c_str())->header.center;
	Vector3 pos = m * center;

	Mesh* mesh = Mesh::Get(this->mesh.c_str());

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && culling)
		return;

	Vector3 toPlane = pos - camera->eye;
	float distToPlane = toPlane.length();

	if (distToPlane > 1500)
	{
		this->mesh = "spitfire_low_flat.ASE";
		this->texture = "data/textures/spitfire_low_flat.tga";
	}
	else if (distToPlane > 1000)
	{
		this->mesh = "spitfire_low.ASE";
	}
	else
	{
		this->mesh = originals[0];
		this->texture = originals[1];
	}


	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	for (int i = 0; i < this->children.size(); i++)
	{
		this->children[i]->render(camera);
	}
}

void Airplane::update(float elapsed_time)
{
	if (decoration == 1)
		return;

	// IA
	if (controller != NULL)
	{
		controller->setTarget(World::getInstance()->playerAir->getPosition());
		controller->update(elapsed_time);
	}
	// NOT IA
	else
	{
		testSphereCollision();
		testStaticCollisions();
	}

	if (getPosition().y > 10.f && wheels_rotation < 7.5)
	{
		//std::cout << getPosition().y << std::endl;
		wheels_rotation += elapsed_time;
		children[0]->model.rotateLocal(elapsed_time*0.2, Vector3(0, 0, 1));
		children[1]->model.rotateLocal(elapsed_time*0.2, Vector3(0, 0, -1));
	}

	//  cooling to overheat
	if (overused)
		timer += elapsed_time;

	float time_cooling = 5.0;

	if (timer > time_cooling)
	{
		timer = shootingtime = 0;
		overused = false;
	}
}

void Airplane::shoot()
{
	BulletManager* bManager = BulletManager::getInstance();
	Game*game = Game::getInstance();

	if (getTime() > last_shoot && !overused)
	{
		Vector3 vel = model.rotateVector(Vector3(0.f, 0.f, 1500));

		//std::cout << "shooting" << std::endl;
		
		switch (planeModel) {
		case SPITFIRE:
			bManager->createBullet(model*Vector3(1.9f, -0.25f, 2.0), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-2.f, -0.25f, 2.0), vel, 2, damageM60, this, 1);
			Flash::createFlash(Vector3(1.95f, -0.26f, 1.75), this);
			Flash::createFlash(Vector3(-2.0f, -0.26f, 1.75), this);
			break;
		case P38:
			bManager->createBullet(model*Vector3(0.5f, -0.25f, 10.f), vel, 1, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-0.5f, -0.25f, 10.f), vel, 1, damageM60, this, 1);
			bManager->createBullet(model*Vector3(0.f, -0.1f, 10.f), vel, 1, damageM60, this, 1);
			Flash::createFlash(Vector3(0.5f, -0.35f, 6.f), this);
			Flash::createFlash(Vector3(-0.5f, -0.35f, 6.f), this);
			Flash::createFlash(Vector3(0.f, -0.25f, 6.f), this);
			break;
		case WILDCAT:
			bManager->createBullet(model*Vector3(-2.25f, -0.50f, 3.f), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(2.25f, -0.50f, 3.f), vel, 2, damageM60, this, 1);
			Flash::createFlash(Vector3(-2.25f, -0.50f, 2.75f), this);
			Flash::createFlash(Vector3(2.25f, -0.50f, 2.75f), this);
			break;
		case BOMBER:
			bManager->createBullet(model*Vector3(2.40f, -0.25f, 5.f), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-2.55f, -0.25f, 5.f), vel, 2, damageM60, this, 1);
			Flash::createFlash(Vector3(2.40f, -0.25f, 5.f), this);
			Flash::createFlash(Vector3(-2.55f, -0.25f, 5.f), this);
			break;
		}

		SoundManager::getInstance()->playSound("shot", false);

		// end shoot

		last_shoot = getTime() + (1000 - cadence * 10);

		if (!overused)
			shootingtime += 2;

		if (!overused && shootingtime == 50)
		{
			overused = true;
			shootingtime = 0;
		}

		//std::string s = overused ? "ON" : "OFF";
		//std::cout << "s_time: " << shootingtime << " - overused " << s << std::endl;
	}
}

void Airplane::rear_shoot()
{
	BulletManager* bManager = BulletManager::getInstance();
	Game*game = Game::getInstance();

	if (getTime() > last_shoot)
	{
		Camera* camtest = game->shooter_camera; 
		Vector3 vel = camtest->getRayDirection(game->mouse_position.x, game->mouse_position.y, game->window_width, game->window_height);
		vel *= 1500;

		switch (planeModel) {
		case SPITFIRE:
			bManager->createBullet(model*Vector3(1.f, -0.25f, 0.0), vel, 2, 10, this, 2);
			Flash::createFlash(Vector3(0.f, 0.f, -8.f), this, 5.0);
			break;
		case P38:
			bManager->createBullet(model*Vector3(0.5f, -0.25f, 0.f), vel, 1, 10, this, 2);
			Flash::createFlash(Vector3(0.f, 0.f, -8.f), this, 5.0);
			break;
		case WILDCAT:
			bManager->createBullet(model*Vector3(1.25f, -0.50f, 0.f), vel, 2, 10, this, 2);
			Flash::createFlash(Vector3(0.f, 0.f, -8.f), this, 5.0);
			break;
		}
		
		SoundManager::getInstance()->playSound("shot", false);

		// end shoot
		last_shoot = getTime() + 100;
	}
}

void Airplane::createTorpedos()
{
	Torpedo * t1 = new Torpedo(NO_CULLING);
	Torpedo * t2 = new Torpedo(NO_CULLING);
	t1->name = "torpedo1";
	t2->name = "torpedo2";

	switch (planeModel) {
	case SPITFIRE:
		t1->model.traslate(0.75f, -0.75f, -0.5f);
		t2->model.traslate(-0.75f, -0.75f, -0.5f);
		break;
	case P38:
		t1->model.traslate(1.25f, -0.3f, -0.5f);
		t2->model.traslate(-1.25f, -0.3f, -0.5f);
		break;
	case WILDCAT:
		t1->model.traslate(0.5f, -0.75f, -0.5f);
		t2->model.traslate(-0.5f, -0.75f, -0.5f);
		break;
	case BOMBER:
		t1->model.traslate(1.5f, -1.75f, 0.25f);
		t2->model.traslate(-1.5f, -1.75f, 0.25f);
		break;
	}

	torpedos[0] = t1;
	torpedos[1] = t2;

	torpedosLeft = 2;

	addChild(t1);
	addChild(t2);
}

void Airplane::torpedoShoot() {

	if (!torpedosLeft)
		return;

	torpedosLeft--;

	SoundManager::getInstance()->playSound("missil", false);

	for (int i = 0; i < 2; i++)
	{
		if (torpedos[i] != NULL)
		{
			torpedos[i]->activate();
			torpedos[i] = NULL;
			return;
		}
	}
}

void Airplane::onCollision(EntityCollider* collided_with)
{
	Game* game = Game::getInstance();

	//std::cout << "colliding" << std::endl;

	if (uid > 1000)
		return;

	if (collided_with == NULL)
	{
		std::cout << "CRASCHING WITH SHIPS OR NATURE!" << std::endl;
		life = 0;
		return;
	}

	// collided with not NULL

	if (collided_with->uid > 500 && collided_with->uid < 505)
	{
		// collided with powerup
		PowerUp * pup = (PowerUp*)collided_with;
		pup->execute();
		return;
	}
	
	std::cout << "CRASCHING WITH AIRPLANES!" << std::endl;
	life = 0;
}

void Airplane::unboundController()
{
	if (controller->player != NULL)
		controller->player = NULL;
	if (controller != NULL)
		controller = NULL;
}

// *************************************************************************
// SHIP
// *************************************************************************

Ship::Ship(bool ia)
{
	setName("ship");

	std::string shader = ia ? "color" : "simple";

	set("barco.ASE", "data/textures/barco.tga", shader.c_str());
	setStatic();
	setDynamic();

	if (ia)
	{
		setUid(Airplane::ENEMY_SHIP);
		setLife(2000);
		model.setTranslation(2000, -10, 1700);
	}
	else
	{
		setUid(Airplane::PLAYER_SHIP);
		model.setRotation(180 * DEG2RAD, Vector3(0.f, 1.f, 0.f));
		model.traslate(1600, -10, 1700);
		setLife(500);
	}

	EntityMesh* turretOne = new EntityMesh();

	turretOne->set("barco_turret.ASE", "data/textures/barco_turret.tga", shader.c_str());
	turretOne->model.setRotation(90 * DEG2RAD, Vector3(0.f, 1.f, 0.f));
	turretOne->model.traslate(0.f, 5.25f, 66.f);
	addChild(turretOne);

	EntityMesh* turretTwo = new EntityMesh();

	turretTwo->set("barco_turret.ASE", "data/textures/barco_turret.tga", shader.c_str());
	turretTwo->model.setRotation(90 * DEG2RAD, Vector3(0.f, 1.f, 0.f));
	turretTwo->model.traslate(0, 8, -63);
	addChild(turretTwo);

	EntityMesh* cannonOne = new EntityMesh();

	cannonOne->set("barco_cannons.ASE", "data/textures/barco_turret.tga", shader.c_str());
	cannonOne->model.traslate(0.f, 0.01f, 0.f);
	turretOne->addChild(cannonOne);

	EntityMesh* cannonTwo = new EntityMesh();

	cannonTwo->set("barco_cannons.ASE", "data/textures/barco_turret.tga", shader.c_str());
	cannonTwo->model.traslate(0.f, 0.01f, 0.f);
	turretTwo->addChild(cannonTwo);

	// ship properties

	last_shoot = getTime() + 5000;
	cannonReady = false;
}

Ship::~Ship()
{
	World* world = World::getInstance();

	std::vector<Entity*>::iterator it;
	it = std::find(world->ships.begin(), world->ships.end(), this);

	if (it != world->ships.end())
		world->ships.erase(it);
}

//meshfile sin path, texturefile con path
void Ship::set(const char * meshf, const char * texturef, const char * shaderf)
{
	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void Ship::render(Camera * camera)
{
	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 center = Mesh::Get(mesh.c_str())->header.center;
	Vector3 pos = m * center;

	Mesh* mesh = Mesh::Get(this->mesh.c_str());

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && this->culling)
		return;

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	for (int i = 0; i < this->children.size(); i++)
	{
		this->children[i]->render(camera);
	}
}

void Ship::update(float elapsed_time)
{
	if (!Game::instance->start)
		return;
	
	// each ship has a different probability of shooting 
	
	int r;
	if (uid == Airplane::ENEMY_SHIP) // 1/3
		r = rand() % 3;
	else if (uid == Airplane::PLAYER_SHIP) // 1/4
		r = rand() % 4;

	if (getTime() > last_shoot)
	{
		//std::cout << r << std::endl;
		if (r == 1)
			shoot();
		last_shoot = getTime() + 3000;
	}
}


void Ship::shoot()
{
	//if (uid == Airplane::ENEMY_SHIP)
	//	return;

	//std::string a = uid == Airplane::ENEMY_SHIP ? "Enemy" : "player ";
	//std::cout << a  << " shooting" << std::endl;

	SoundManager::getInstance()->playSound("cannonship", false);

	Missile * missile = new Missile(NO_CULLING);
	missile->setUid(uid);

	if(cannonReady)
		missile->model.traslate(20.f, 5.25f, 66.f);
	else 
		missile->model.traslate(20.f, 8.f, -63.f);

	//std::cout << angleDisp << std::endl;

	this->addChild(missile);
	missiles.push_back(missile);
	missile->activate();

	Explosion::createExplosion(missile->getPosition(), 7.5);

	cannonReady = !cannonReady;
}

void Ship::onCollision(EntityCollider* collided_with)
{
	std::cout << "si sale esto, algo no va bien" << std::endl; 
}

// *************************************************************************
// AIRCARRIER
// *************************************************************************

Aircarrier::Aircarrier(int i, int j)
{
	setName("aircarrier");
	std::string shader = "simple";
	set("aircarrier.ASE", "data/textures/aircarrier_metal.tga"
		,"data/textures/aircarrier_wood.tga"
		, "simple");

	model.setTranslation(2000 + i, -10, -2000 + j);
	setStatic();
}

Aircarrier::~Aircarrier()
{
	
}

void Aircarrier::set(const char * meshf, const char * texture1, const char * texture2, const char * shaderf)
{
	mesh = meshf;
	texture = texture1;
	this->texture2 = texture2;
	materialTriangle = 15186 * 3;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void Aircarrier::render(Camera * camera)
{
	Mesh* thismesh = Mesh::Get(this->mesh.c_str());
	unsigned int size = thismesh->vertices.size();

	//
	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 center = Mesh::Get(mesh.c_str())->header.center;
	Vector3 pos = m * center;

	Mesh* mesh = Mesh::Get(this->mesh.c_str());

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && this->culling)
		return;

	if (alpha)
	{
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	}

	if (!depthTest)
		glDisable(GL_DEPTH_TEST);
	if (!depthMask)
		glDepthMask(GL_FALSE);
	if (!cullFace)
		glDisable(GL_CULL_FACE);

	// MULTIMATERIAL

	renderDifMaterials(camera, 0, materialTriangle, texture.c_str());
	renderDifMaterials(camera, materialTriangle, size, texture2.c_str());
	
	//

	if (!cullFace)
		glEnable(GL_CULL_FACE);

	if (!depthTest)
		glEnable(GL_DEPTH_TEST);
	if (!depthMask)
		glDepthMask(GL_TRUE);

	if (alpha)
	{
		glDisable(GL_BLEND);
	}

	for (int i = 0; i < this->children.size(); i++)
	{
		this->children[i]->render(camera);
	}
}


// *************************************************************************
// TORPEDO
// *************************************************************************

Torpedo::Torpedo(bool culling)
{
	parent = NULL;
	model.setRotation(180 * DEG2RAD, Vector3(0.f, 1.f, 0.f));

	mesh = "torpedo.ASE";
	texture = "data/textures/torpedo.tga";
	std::string shader_string = "texture";
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	shader = Shader::Load(vs.c_str(), fs.c_str());

	ready = false;
	max_ttl = 4.0;
	ttl = max_ttl;
}

Torpedo::~Torpedo()
{

}

void Torpedo::update(float elapsed_time)
{
	if (!ready)
		return;

	if (ttl < 0)
	{
		destroy();
		return;
	}

	testSphereCollision();

	last_position = getPosition();

	model.traslateLocal(0, 0, (max_ttl - 0.8 * ttl) * elapsed_time * -75);
	ttl -= elapsed_time;
	Smoke::createSmoke(model * Vector3(0, 0, -2.5), this);
	Smoke::createSmoke(model * Vector3(0, 0, -2.25), this);
	Smoke::createSmoke(model * Vector3(0, 0, -2.0), this);
}

void Torpedo::activate()
{
	Entity* root = World::getInstance()->root;
	Matrix44 mod = this->getGlobalMatrix();

	this->parent->removeChild(this);
	root->addChild(this);
	this->model = mod;

	ready = true;
}

void Torpedo::onCollision(EntityCollider* collided_with)
{
	if (collided_with == World::getInstance()->playerAir)
		return;

	//std::cout << "torpedo ha colisionado" << std::endl;
	
	Explosion::createExplosion(collided_with->getPosition());
	Explosion::createExplosion(collided_with->getPosition());
	SoundManager::getInstance()->playSound("explosion", false);

	collided_with->life -= 450.0;

	collided_with->life = max(collided_with->life, 0);

	Game::instance->score += 500.0;

	// shot a plane with a torpedo, yeah!!
	if(collided_with->uid > 1000)
		Game::instance->score += 500.0;
	
	if (!collided_with->life)
	{
		//std::cout << collided_with->name << " destroyed by torpedo" << std::endl;
		collided_with->destroy();
	}

	// destruir torpedo
	ttl = -1.0;
}

// *************************************************************************
// MISSILE
// *************************************************************************

Missile::Missile(bool culling)
{
	parent = NULL;
	setName("missile");
	model.setRotation(90 * DEG2RAD, Vector3(0.f, 1.f, 0.f));

	mesh = "bomb.ASE";
	texture = "data/textures/bomb.tga";
	std::string shader_string = "texture";

	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	shader = Shader::Load(vs.c_str(), fs.c_str());

	max_ttl = 3.5;
	ttl = max_ttl;
	hasCollided = false;
}

Missile::~Missile()
{
	rastro.clear();
}

void Missile::render(Camera* camera)
{
	EntityMesh::render(camera);

	rastro.vertices.push_back(getPosition());
	rastro.vertices.push_back(last_position);

	float alpha = min((1 - (ttl / max_ttl) ), 0.5);
	alpha *= 0.6;

	if (uid == Airplane::ENEMY_SHIP)
	{
		rastro.colors.push_back(Vector4(1.f, 0.15, 0.f, alpha));
		rastro.colors.push_back(Vector4(1.f, 0.15, 0.f, alpha));
	}
	else
	{
		rastro.colors.push_back(Vector4(0.f, 0.15, 1.f, alpha));
		rastro.colors.push_back(Vector4(0.f, 0.15, 1.f, alpha));
	}
		
	if (!rastro.vertices.size())
		return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float size = camera->getProjectScale(getPosition(), 50.0);
	glLineWidth(size*0.1);
	rastro.render(GL_LINES);
	glDisable(GL_BLEND);
	glLineWidth(1.0);
}

void Missile::update(float elapsed_time)
{
	if (ttl < 0)
	{
		if(!hasCollided)
			std::cout << "missil no ha colisionado (destroy by ttl)" << std::endl;
		destroy();
		return;
	}

	testSphereCollision();
	last_position = getPosition();

	model.traslateLocal(0, 0, elapsed_time * 150.0);
	ttl -= elapsed_time;
}

void Missile::activate()
{
	Entity* root = World::getInstance()->root;
	Matrix44 mod = this->getGlobalMatrix();

	this->parent->removeChild(this);
	root->addChild(this);
	this->model = mod;
}

void Missile::onCollision(EntityCollider* collided_with)
{
	if (collided_with == PlayerController::getInstance()->player)
		return;

	Explosion::createExplosion(collided_with->model * Vector3(0, 15, random() * 10), 20.0);
	Explosion::createExplosion(collided_with->model * Vector3(0, 15, -random() * 10), 20.0);
	SoundManager::getInstance()->playSound("explosion", false);

	collided_with->life -= random()*25.0;

	if (collided_with->life <= 0)
	{
		std::cout << collided_with->name << " destroyed by torpedo" << std::endl;
		collided_with->destroy();
	}

	// destruir torpedo
	hasCollided = true;
	ttl = -1.0;
}

// *************************************************************************
// HELIX
// *************************************************************************

Helix::Helix()
{
	alpha = true;
	depthMask = false;
	cullFace = false;
	set("helice.ASE", "data/textures/helice.tga", "plane");
}

Helix::~Helix()
{

}

// *************************************************************************
// CLOUDS
// *************************************************************************

Clouds::Clouds()
{
	mesh = "plane";
	shader = Shader::Load("data/shaders/cloud.vs", "data/shaders/cloud.fs");
	texture = "data/textures/cloud.TGA";
	alpha = true;
	depthMask = false;
	cullFace = false;

	clouds.resize(200);

	for (int i = 0; i < clouds.size(); i++)
	{
		Vector3 pos;
		pos.random(Vector3(5500, 2000, 5500));
		pos.y += 2500;
		clouds[i].pos = pos;
		clouds[i].size = 3000 + random()*1000;
	}
}

bool mySort(Clouds::sCloudInfo &a, Clouds::sCloudInfo &b)
{
	return a.distance > b.distance;
}

void Clouds::render(Camera* camera)
{
	Mesh m;
	Airplane* player = PlayerController::getInstance()->player;

	Vector3 up = Vector3(0, 1, 0);
	Vector3 right = (camera->center - camera->eye).cross(up);
	right.normalize();

	for (int i = 0; i < clouds.size(); i++)
	{
		sCloudInfo& c = clouds[i];
		c.distance = c.pos.distance(camera->eye);
	}

	// ordenar por distancia a la camara
	std::sort(clouds.begin(), clouds.end(), mySort);

	for (int i = 0; i < clouds.size(); i++)
	{
		sCloudInfo& c = clouds[i];

		m.vertices.push_back(c.pos - right * c.size * 0.5 + c.pos + up * c.size * 0.5);
		m.vertices.push_back(c.pos + right * c.size * 0.5 + c.pos + up * c.size * 0.5);
		m.vertices.push_back(c.pos - right * c.size * 0.5 + c.pos - up * c.size * 0.5);

		m.vertices.push_back(c.pos + right * c.size * 0.5 + c.pos + up * c.size * 0.5);
		m.vertices.push_back(c.pos + right * c.size * 0.5 + c.pos - up * c.size * 0.5);
		m.vertices.push_back(c.pos - right * c.size * 0.5 + c.pos - up * c.size * 0.5);

		m.uvs.push_back(Vector2(0, 1));
		m.uvs.push_back(Vector2(1, 1));
		m.uvs.push_back(Vector2(0, 0));

		m.uvs.push_back(Vector2(1, 1));
		m.uvs.push_back(Vector2(1, 0));
		m.uvs.push_back(Vector2(0, 0));
	}

	if (alpha)
	{
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	}

	if (!depthTest)
		glDisable(GL_DEPTH_TEST);
	if (!depthMask)
		glDepthMask(GL_FALSE);
	if (!cullFace)
		glDisable(GL_CULL_FACE);

	assert(shader);

	shader->enable();
	shader->setMatrix44("u_model", model);
	shader->setMatrix44("u_mvp", camera->viewprojection_matrix);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	shader->setFloat("u_time", Game::getInstance()->time);
	shader->setVector3("u_camera_pos", Game::getInstance()->current_camera->eye);

	m.render(GL_TRIANGLES, shader);
	
	shader->disable();

	if (!cullFace)
		glEnable(GL_CULL_FACE);

	if (!depthTest)
		glEnable(GL_DEPTH_TEST);
	if (!depthMask)
		glDepthMask(GL_TRUE);

	if (alpha)
	{
		glDisable(GL_BLEND);
	}
}

// *************************************************************************
// POWER UPS
// *************************************************************************

bool PowerUp::damageTaken = false;
bool PowerUp::ninjaTaken = false;

PowerUp::PowerUp(const std::string&  type)
{
	setName(type);

	std::string shader = "simple";

	set("box.ASE", "data/textures/bomb.tga", shader.c_str());
	setDynamic();

	if (type == "damage")
	{
		setUid(DAMAGE);
		model.setTranslation(-450, 100, 1500);
	}
	else if (type == "ninja")
	{
		setUid(NINJA);
		model.setTranslation(-2250, 120, -1000);
	}
	else if (type == "bomb")
	{
		setUid(BOMB);
		model.setTranslation(-3600, 120, -3000);
	}

}

PowerUp::~PowerUp()
{
	World* world = World::getInstance();

	std::vector<Entity*>::iterator it;
	it = std::find(world->powerups.begin(), world->powerups.end(), this);

	if (it != world->powerups.end())
		world->powerups.erase(it);
}

//meshfile sin path, texturefile con path
void PowerUp::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void PowerUp::execute()
{
	if (uid == PowerUp::DAMAGE)
	{
		damageTaken = true;
		//std::cout << "DAMAGE x2" << std::endl;
		World::instance->playerAir->damageM60 *= 2;
		destroy();
		return;
	}

	else if (uid == PowerUp::NINJA)
	{
		ninjaTaken = true;
		//std::cout << "NINJA" << std::endl;
		World::instance->playerAir->visibility = 125;
		destroy();
		return;
	}
	else if (uid == PowerUp::BOMB)
	{
		//std::cout << "BOMB" << std::endl;
		destroy();
		World::instance->atomic_enabled = true;
		return;
	}
}

void PowerUp::reset()
{
	damageTaken = false;
	ninjaTaken = false;
}

void PowerUp::update(float elapsed_time)
{
	
}

void PowerUp::onCollision(EntityCollider* collided_with)
{
	std::cout << "si sale esto, algo no va bien" << std::endl;
}