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
#include "world.h"
#include "bass.h"
#include "mesh.h"

// *************************************************************************
// AIRPLANE 
// *************************************************************************

Airplane::Airplane(int model, bool culling) {

	EntityCollider* wh_right = new EntityCollider();
	wh_right->set("spitfire_wheel_right.ASE", "data/textures/spitfire.tga", "plane");

	EntityCollider* wh_left = new EntityCollider();
	wh_left->set("spitfire_wheel_left.ASE", "data/textures/spitfire.tga", "plane");

	Helix* helix = new Helix();

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
	}

	else if (model == P38)
	{
		set("p38.ASE", "data/textures/p38.tga", "plane");
		helix->model.setTranslation(2.44f, 0.f, 2.85f);
		
		Helix* helix2 = new Helix();
		helix2->model.setTranslation(-2.44f, 0.f, 2.85f);
		helix2->model.rotateLocal(3.1415, Vector3(0, 1, 0));
		this->addChild(helix2);
	}

	else if (model == WILDCAT)
	{
		set("wildcat.ASE", "data/textures/wildcat.tga", "plane");
		helix->model.setTranslation(0.f, 0.f, 3.1f);
	}

	else if (model == BOMBER)
	{
		set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "plane");
		helix->model.setTranslation(2.65f, -0.88f, 4.65f);

		Helix* helix2 = new Helix();
		helix2->model.setTranslation(-2.65f, -0.88f, 4.65f);
		helix2->model.rotateLocal(3.1415, Vector3(0, 1, 0));
		this->addChild(helix2);
	}

	helix->model.rotateLocal(3.1415, Vector3(0, 1, 0));
	this->addChild(helix);

	// plane properties
	engine = false;
	wheels_rotation = 0;
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

	BulletManager::getInstance()->update(elapsed_time);
	Entity::update(elapsed_time);

	testSphereCollision();
	testStaticCollisions();

	if (getPosition().y > 10.f && wheels_rotation < 7.5)
	{
		//std::cout << getPosition().y << std::endl;
		wheels_rotation += elapsed_time;
		children[0]->model.rotateLocal(elapsed_time*0.2, Vector3(0, 0, 1));
		children[1]->model.rotateLocal(elapsed_time*0.2, Vector3(0, 0, -1));
	}
}

void Airplane::m60Shoot() {
	BulletManager* bManager = BulletManager::getInstance();
	Game*game = Game::getInstance();
	int planeModel = World::getInstance()->worldInfo.playerModel;

	Vector3 cannon_pos1;
	Vector3 cannon_pos2;
	Vector3 cannon_pos3;
	switch (planeModel) {
	case SPITFIRE:
		cannon_pos1 = Vector3(1.9f, -0.25f, 2.f);
		cannon_pos2 = Vector3(-1.9f, -0.25f, 2.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1.5, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1.5, this->damageM60, this, 1);
		break;
	case P38:
		cannon_pos1 = Vector3(0.5f, -0.25f, 10.f);
		cannon_pos2 = Vector3(-0.5f, -0.25f, 10.f);
		cannon_pos3 = Vector3(0.f, -0.1f, 10.f);

		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos3, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
		break;
	case WILDCAT:
		cannon_pos1 = Vector3(0.f, -0.50f, 10.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 2, this->damageM60, this, 1);
		break;
	case BOMBER:
		cannon_pos1 = Vector3(2.40f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-2.55f, -0.25f, 5.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 2, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 2, this->damageM60, this, 1);
		break;
	}

	int sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it
}

void Airplane::createTorpedos()
{
	Torpedo * t1 = new Torpedo(NO_CULLING);
	Torpedo * t2 = new Torpedo(NO_CULLING);

	switch (World::getInstance()->worldInfo.playerModel) {
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

	int sample = BASS_SampleLoad(false, "data/sounds/missil.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it

	for (int i = 0; i < 2; i++)
	{
		if (!torpedos[i]->ready)
		{
			torpedos[i]->activate();
			return;
		}
	}
}

void Airplane::onCollision(EntityCollider* collided_with) {
	//Game* game = Game::getInstance();
	std::cout << "CRASHED!" << std::endl;
	exit(1);
	//game->sManager->changeCurrentState(EndingState::getInstance(game->sManager));
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

	ttl = 4;
}

Torpedo::~Torpedo() {}

void Torpedo::update(float elapsed_time) {

	if (!ready) return;

	if (this->ttl < 0) {
		destroy();
		return;
	}

	testSphereCollision();

	model.traslateLocal(0, 0, (4 - ttl) * elapsed_time * -150 * 0.5);
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
	int b_sample = BASS_SampleLoad(false, "data/sounds/explosion.wav", 0L, 0, 1, 0);
	HCHANNEL hSampleChannel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
	BASS_ChannelPlay(hSampleChannel, false); // play it

	collided_with->life -= 100;
	collided_with->life = max(collided_with->life, 0);

	// destruir current enemy
	if (!collided_with->life) {
		collided_with->destroy();
	}

	// destruir torpedo
	destroy();
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