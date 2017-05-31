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
#include "world.h"
#include "bass.h"
#include "mesh.h"
#include <algorithm>
#include <cassert>

// *************************************************************************
// AIRPLANE 
// *************************************************************************

Airplane::Airplane(int model, IAController* controller, bool culling) {

	planeModel = model;
	state = "waypoints";

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
		wh_right->model.rotateLocal(1.57, Vector3(0, 0, -1));
		this->addChild(wh_right);

		wh_left->model.setTranslation(0.82, -0.58, 0.16);
		wh_left->model.rotateLocal(1.57, Vector3(0, 0, 1));
		this->addChild(wh_left);

		life = 175.0;
		cadence = 75.0;
		damageM60 = 100.0;
		speed = 100.0;
	}

	else if (model == P38)
	{
		set("p38.ASE", "data/textures/p38.tga", "plane");
		helix->model.setTranslation(2.44f, 0.f, 2.85f);
		
		Helix* helix2 = new Helix();
		helix2->setName("helix_2");
		helix2->model.setTranslation(-2.44f, 0.f, 2.85f);
		helix2->model.rotateLocal(3.1415, Vector3(0, 1, 0));
		this->addChild(helix2);

		life = 200.0;
		cadence = 85.0;
		damageM60 = 5.0;
		speed = 90.0;
	}

	else if (model == WILDCAT)
	{
		set("wildcat.ASE", "data/textures/wildcat.tga", "plane");
		helix->model.setTranslation(0.f, 0.f, 3.1f);

		life = 300.0;
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
		helix2->model.rotateLocal(3.1415, Vector3(0, 1, 0));
		this->addChild(helix2);

		life = 350.0;
		cadence = 55.0;
		damageM60 = 15.0;
		speed = 80.0;
	}

	helix->model.rotateLocal(3.1415, Vector3(0, 1, 0));
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

	if (controller != NULL)
	{
		this->controller = controller;
		controller->setPlayer(this);
	}
}

Airplane::~Airplane() {}

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

		int sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
		int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
		BASS_ChannelPlay(channel, false); // play it

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

	int sample = BASS_SampleLoad(false, "data/sounds/missil2.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it

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
		exit(1);
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
// TORPEDO
// *************************************************************************

Torpedo::Torpedo(bool culling) {
	parent = NULL;
	model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));

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

	if (ttl < 0) {
		destroy();
		return;
	}

	testSphereCollision();

	model.traslateLocal(0, 0, (max_ttl - ttl) * elapsed_time * -150 * 0.5);
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

void Torpedo::onCollision(EntityCollider* collided_with) {

	if (collided_with == PlayerController::getInstance()->player)
		return;

	int b_sample = BASS_SampleLoad(false, "data/sounds/explosion.wav", 0L, 0, 1, 0);
	HCHANNEL hSampleChannel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
	BASS_ChannelPlay(hSampleChannel, false); // play it

	collided_with->onCollision(this);

	// destruir torpedo
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

// **************************************************************************************

Clouds::Clouds()
{
	mesh = "plane";
	shader = Shader::Load("data/shaders/cloud.vs", "data/shaders/cloud.fs");
	texture = "data/textures/cloud.TGA";
	alpha = true;
	depthMask = false;
	cullFace = false;

	clouds.resize(100);

	for (int i = 0; i < clouds.size(); i++)
	{
		Vector3 pos;
		pos.random(Vector3(15000, 10000 + random() * 1000, 15000));
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

	Vector3 up = Vector3(0, 1, 0);
	Vector3 right = Vector3(1, 0, 0);

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

	/*Vector3 to_player;
	float angle;

	model.rotateLocal(angle, up);

	*/

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