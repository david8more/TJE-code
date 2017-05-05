#include "states/state.h"
#include "states/playstate.h"
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
#include <cassert>
#include <algorithm>

// ENTITY

std::vector<Entity*> Entity::destroy_pending;

Entity::Entity() {
	parent = NULL;
}

Entity::~Entity() {}

Vector3 Entity::getPosition() { 
	return model * Vector3();
}

void Entity::render(Camera * camera) {
	for (int i = 0; i < children.size(); i++) {
		if (children[i]->name == "RELOAD_ZONE") {
			glColor4f(1.0, 1.0, 1.0, 1.0);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			children[i]->render(camera);
			glDisable(GL_BLEND);
		}
		children[i]->render(camera);
	}
}

void Entity::update(float elapsed_time) {
	for (int i = 0; i < children.size(); i++) {
		children[i]->update(elapsed_time);
	}
}

void Entity::addChild(Entity * entity) {
	if (entity->parent) {
		return;
	}
	entity->parent = this;
	children.push_back( entity );
}

void Entity::removeChild(Entity* entity) {

	auto it = std::find(children.begin(), children.end(), entity);
	if (it == children.end()) return;

	children.erase(it);
	entity->parent = NULL;
	entity->model = entity->model * this->getGlobalMatrix();
}

Matrix44 Entity::getGlobalMatrix() {
	if( parent ) return model * parent->getGlobalMatrix();
	else return model;
}

void Entity::destroy(){
	destroy_pending.push_back(this);
	for (int i = 0; i < children.size(); i++) {
		children[i]->destroy();
	}
}

void Entity::destroy_entities() {

	for (int i = 0; i < destroy_pending.size(); i++) {
		Entity* ent = destroy_pending[i];
		ent->parent->removeChild(ent);
		ent->parent = NULL;
		delete(ent);

	}
	destroy_pending.clear();
}

// *************************************************************************
// ENTITYMESH 
// *************************************************************************

EntityMesh::EntityMesh(bool culling) {
	this->culling = culling;
}
EntityMesh::~EntityMesh() {}

//meshfile sin path, texturefile con path
void EntityMesh::set(const char * meshf, const char * texturef, const char * shaderf ) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void EntityMesh::render(Camera * camera) {
	
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

	// no pintar helice
	if (this->name == "NO_CHILD") return;
			
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->render(camera);
	}
}

void EntityMesh::update( float elapsed_time ) {

	
}

// *************************************************************************
// ENTITYCOLLIDER
// *************************************************************************

EntityCollider::EntityCollider() {}
EntityCollider::~EntityCollider() {}

std::vector<EntityCollider*> EntityCollider::static_colliders;
std::vector<EntityCollider*> EntityCollider::dynamic_colliders;

void EntityCollider::setStatic() {
	static_colliders.push_back(this);
	is_static = true;
}

void EntityCollider::setDynamic() {
	dynamic_colliders.push_back(this);
	is_dynamic = true;
}

bool EntityCollider::testRayWithAll(Vector3 origin, Vector3 dir, float max_dist, Vector3& collisions) {

	for (int j = 0; j < static_colliders.size(); j++) {

		EntityEnemy * current_enemy = (EntityEnemy*)static_colliders[j];

		//si queremos especificar la model de la mesh usamos setTransform

		Mesh* enemy_mesh = Mesh::Get(current_enemy->mesh.c_str());
		CollisionModel3D * collisionModel = enemy_mesh->getCollisionModel();

		collisionModel->setTransform(current_enemy->model.m);

		//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
		//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
		//al origen del rayo o nos conformamos con saber si colisiona. 

		if (!collisionModel->rayCollision(origin.v, dir.v, true))
			continue;

		collisionModel->getCollisionPoint(collisions.v, false);

		current_enemy->onBulletCollision();

		return true;
	}

	return false;

}

void EntityCollider::onBulletCollision() {}

// *************************************************************************
// ENTITYPLAYER 
// *************************************************************************

EntityPlayer::EntityPlayer(bool culling) {

}

EntityPlayer::~EntityPlayer() {}

//meshfile sin path, texturefile con path
void EntityPlayer::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void EntityPlayer::render(Camera * camera) {

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

void EntityPlayer::update(float elapsed_time) {

	BulletManager::getInstance()->update(elapsed_time);
	Entity::update(elapsed_time);
}

void EntityPlayer::m60Shoot() {
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

void EntityPlayer::createTorpedos()
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

void EntityPlayer::torpedoShoot() {
	
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

void EntityPlayer::onCollision() {
	
}

// *************************************************************************
// ENTITYENEMY
// *************************************************************************

EntityEnemy::EntityEnemy(bool culling) {
	
}
EntityEnemy::~EntityEnemy() {}

//meshfile sin path, texturefile con path
void EntityEnemy::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void EntityEnemy::render(Camera * camera) {

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

	for (int i = 0; i < this->children.size(); i++)
	{
		this->children[i]->render(camera);
	}

}

void EntityEnemy::update(float elapsed_time) {
	
}

void EntityEnemy::onBulletCollision() {

	this->life -= 5;
	this->life = max(this->life, 0);

	if (this->life == 0) World::getInstance()->root->removeChild(this);
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