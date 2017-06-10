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
#include <algorithm>
#include <cassert>

// *************************************************************************
// AIRPLANE 
// *************************************************************************

Airplane::Airplane(int model, bool ia, bool culling) {

	planeModel = model;

	EntityCollider* wh_right = new EntityCollider();
	wh_right->setName("right wheel");
	wh_right->set("spitfire_wheel_right.ASE", "data/textures/spitfire.tga", "plane");

	EntityCollider* wh_left = new EntityCollider();
	wh_left->setName("left wheel");
	wh_left->set("spitfire_wheel_left.ASE", "data/textures/spitfire.tga", "plane");

	Helix* helix = new Helix();
	helix->setName("helix");

	if (model == SPITFIRE)
	{
		set("spitfire.ASE", "data/textures/spitfire.tga", "plane");
		helix->model.setTranslation(0.f, 0.f, 2.1f);

		wh_right->model.setTranslation(-0.82, -0.58, 0.16);
		wh_right->model.rotateLocal(90.0 * DEG2RAD, Vector3(0, 0, -1));
		this->addChild(wh_right);

		wh_left->model.setTranslation(0.82, -0.58, 0.16);
		wh_left->model.rotateLocal(90.0 * DEG2RAD, Vector3(0, 0, 1));
		this->addChild(wh_left);

		setLife(175);
		cadence = 75.0;
		damageM60 = 25.0;
		speed = 100.0;
		if (0) // god mode
		{
			damageM60 = 1000.0;
		}
		if (0) // slow mode
		{
			speed = 20.0;
		}

	}

	else if (model == P38)
	{
		set("p38.ASE", "data/textures/p38.tga", "plane");
		helix->model.setTranslation(2.44f, 0.f, 2.85f);

		Helix* helix2 = new Helix();
		helix2->setName("helix_2");
		helix2->model.setTranslation(-2.44f, 0.f, 2.85f);
		helix2->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
		this->addChild(helix2);

		setLife(200);
		cadence = 85.0;
		damageM60 = 15.0;
		speed = 90.0;
	}

	else if (model == WILDCAT)
	{
		set("wildcat.ASE", "data/textures/wildcat.tga", "plane");
		helix->model.setTranslation(0.f, 0.f, 3.1f);

		setLife(300);
		cadence = 60.f;
		damageM60 = 40.0;
		speed = 70.0;
	}

	else if (model == BOMBER)
	{
		set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "plane");
		helix->model.setTranslation(2.65f, -0.88f, 4.65f);

		Helix* helix2 = new Helix();
		helix2->setName("helix_2");
		helix2->model.setTranslation(-2.65f, -0.88f, 4.65f);
		helix2->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
		this->addChild(helix2);

		setLife(350);
		cadence = 55.0;
		damageM60 = 15.0;
		speed = 80.0;
	}
	else
	{
		exit(0);
	}

	helix->model.rotateLocal(180.0 * DEG2RAD, Vector3(0, 1, 0));
	this->addChild(helix);

	// plane properties
	engine = false;
	wheels_rotation = 0;
	last_shoot = 0;
	//m60 init
	timer = shootingtime = 0;
	overused = false;

	// controller

	this->controller = NULL;

	if (ia)
	{
		this->controller = new IAController();
		controller->setPlayer(this);
	}
}

Airplane::~Airplane()
{
	std::cout << "deleting airplane" << std::endl;

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
void Airplane::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void Airplane::render(Camera * camera) {

	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 center = Mesh::Get(mesh.c_str())->header.center;
	Vector3 pos = m * center;

	Mesh* mesh = Mesh::Get(this->mesh.c_str());

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && this->culling) return;

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->render(camera);
	}
}

void Airplane::update(float elapsed_time) {

	//Entity::update(elapsed_time);

	testSphereCollision();
	testStaticCollisions();

	if (controller != NULL)
	{
		controller->setTarget(World::getInstance()->playerAir->getPosition());
		controller->update(elapsed_time);
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

	if (timer > 5.0)
	{
		timer = shootingtime = 0;
		overused = false;
	}
}

void Airplane::shoot() {
	BulletManager* bManager = BulletManager::getInstance();
	Game*game = Game::getInstance();

	if (getTime() > last_shoot && !overused)
	{
		//std::cout << "shooting" << std::endl;
		Vector3 vel = model.rotateVector(Vector3(0.f, 0.f, 1500));

		switch (planeModel) {
		case SPITFIRE:
			bManager->createBullet(model*Vector3(1.9f, -0.25f, 3.0), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-2.f, -0.25f, 3.0), vel, 2, damageM60, this, 1);
			break;
		case P38:
			bManager->createBullet(model*Vector3(0.5f, -0.25f, 10.f), vel, 1, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-0.5f, -0.25f, 10.f), vel, 1, damageM60, this, 1);
			bManager->createBullet(model*Vector3(0.f, -0.1f, 10.f), vel, 1, damageM60, this, 1);
			break;
		case WILDCAT:
			bManager->createBullet(model*Vector3(-2.25f, -0.50f, 3.f), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(2.25f, -0.50f, 3.f), vel, 2, damageM60, this, 1);
			break;
		case BOMBER:
			bManager->createBullet(model*Vector3(2.40f, -0.25f, 5.f), vel, 2, damageM60, this, 1);
			bManager->createBullet(model*Vector3(-2.55f, -0.25f, 5.f), vel, 2, damageM60, this, 1);
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

	this->addChild(t1);
	this->addChild(t2);
}

void Airplane::torpedoShoot() {

	if (!torpedosLeft)
		return;

	torpedosLeft--;

	SoundManager::getInstance()->playSound("missil2", false);

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

void Airplane::onCollision(EntityCollider* collided_with) {
	//Game* game = Game::getInstance();
	
	if (collided_with != NULL)
	{
		// IA no colisionan entre ellas
		if (uid > 1000 && collided_with->uid > 1000)
		{
			return;
		}
	}
	
	if (name == "player")
	{
		std::cout << "CRASHED!" << std::endl;
		life = 0;
	}

	else if (name == "ia_1")
	{
		std::cout << "IA 1 CRASHED!" << std::endl;
	}

	else if (name == "ia_2")
	{
		std::cout << "IA 2 CRASHED!" << std::endl;
	}
	//game->sManager->changeCurrentState(EndingState::getInstance(game->sManager));
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
		setLife(1000);
		model.setTranslation(2000, -10, 1700);
	}
	else
	{
		setUid(Airplane::PLAYER_SHIP);
		model.setRotation(180 * DEG2RAD, Vector3(0.f, 1.f, 0.f));
		model.traslate(1600, -10, 1700);
		setLife(750);
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
void Ship::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void Ship::render(Camera * camera) {

	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 center = Mesh::Get(mesh.c_str())->header.center;
	Vector3 pos = m * center;

	Mesh* mesh = Mesh::Get(this->mesh.c_str());

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && this->culling) return;

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	for (int i = 0; i < this->children.size(); i++) {
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
// TORPEDO
// *************************************************************************

Torpedo::Torpedo(bool culling) {
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

Torpedo::~Torpedo() {}

void Torpedo::update(float elapsed_time) {

	if (!ready)
		return;

	if (ttl < 0)
	{
		destroy();
		return;
	}

	testSphereCollision();

	model.traslateLocal(0, 0, (max_ttl - 0.8 * ttl) * elapsed_time * -75);
	ttl -= elapsed_time;
	
}

void Torpedo::activate() {

	Entity* root = World::getInstance()->root;
	Matrix44 mod = this->getGlobalMatrix();

	this->parent->removeChild(this);
	root->addChild(this);
	this->model = mod;

	ready = true;
}

void Torpedo::onCollision(EntityCollider* collided_with)
{
	if (collided_with == PlayerController::getInstance()->player)
		return;

	//std::cout << "torpedo ha colisionado" << std::endl;
	
	Explosion::createExplosion(collided_with->model * Vector3());
	Explosion::createExplosion(collided_with->model * Vector3());
	SoundManager::getInstance()->playSound("explosion", false);

	collided_with->life -= 450.0;
	
	if (collided_with->life <= 0)
	{
		std::cout << collided_with->name << " destroyed by torpedo" << std::endl;
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
	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Mesh* mesh = Mesh::Get(this->mesh.c_str());

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

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_normal_texture", Texture::Get("data/textures/normal_water.tga"));
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	shader->setTexture("u_sky_texture", Texture::Get("data/textures/cielo.tga"));
	shader->setFloat("u_time", Game::getInstance()->time);
	shader->setVector3("u_camera_pos", Game::getInstance()->current_camera->eye);
	mesh->render(GL_TRIANGLES, shader);
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

	for (int i = 0; i < this->children.size(); i++)
	{
		this->children[i]->render(camera);
	}

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
	//Explosion::createExplosion(getPosition(), 2.5, 1.0);
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

// **************************************************************************************

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
		pos.random(Vector3(10000, 8000 + random() * 1000, 10000));
		clouds[i].pos = pos;
		clouds[i].size = 3500 + random()*1500;
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

	model.rotateLocal(90 * DEG2RAD, Vector3(0, 1, 0));

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
	shader->setTexture("u_normal_texture", Texture::Get("data/textures/normal_water.tga"));
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	shader->setTexture("u_sky_texture", Texture::Get("data/textures/cielo.tga"));
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