#include "states/state.h"
#include "states/playstate.h"
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

// *************************************************************************
// AIRPLANE 
// *************************************************************************

Airplane::Airplane(bool culling) {

}

Airplane::~Airplane() {}

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
		cannon_pos1 = Vector3(1.9f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-1.9f, -0.25f, 5.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 2, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 2, this->damageM60, this, 1);
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
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
		break;
	case BOMBER:
		cannon_pos1 = Vector3(2.40f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-2.55f, -0.25f, 5.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, this, 1);
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

	t1->setDynamic();
	t2->setDynamic();

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

	if (!torpedosLeft) {
		return;
	}

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

void Airplane::onCollision() {

}

// *************************************************************************
// TORPEDO
// *************************************************************************

Torpedo::Torpedo(bool culling) {
	parent = NULL;
	model.setRotation(3.14159265359f, Vector3(0.f, 1.f, 0.f));

	mesh = "torpedo.ASE";
	texture = "data/textures/torpedo.tga";
	std::string shader_string("simple");
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	shader = Shader::Load(vs.c_str(), fs.c_str());

	ready = false;

	ttl = 3;
}

Torpedo::~Torpedo() {}

void Torpedo::update(float elapsed_time) {

	if (!ready) return;

	if (this->ttl < 0) {
		destroy();
		return;
	}

	model.traslateLocal(0, 0, elapsed_time * -200);
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


// **************************************************************************************