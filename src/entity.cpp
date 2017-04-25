#include "states/state.h"
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
		children[i]->render(camera);
	}
}

void Entity::update(float elapsed_time) {
	for (int i = 0; i < children.size(); i++) {
		children[i]->update(elapsed_time);
	}
}

void Entity::addChild(Entity * entity) {
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
	std::cout << "deleting";
}

void Entity::destroy_entities() {

	/*for (int i = 0; i < destroy_pending.size(); i++) {
		if (destroy_pending[i]->children.size() > 0) {

		}
		destroy_pending[i]->parent = NULL;
		delete(destroy_pending[i]);
	}*/
	
}

// *************************************************************************
// ENTITYMESH 
// *************************************************************************

EntityMesh::EntityMesh() {
	mesh = NULL;
	parent = NULL;
	texture = NULL;
	shader = NULL;
}
EntityMesh::~EntityMesh() {}

//meshfile sin path, texturefile con path
void EntityMesh::set(const char * meshf, const char * texturef, const char * shaderf ) {

	mesh = Mesh::Get(meshf);
	texture = Texture::Get(texturef);

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
	assert(shader);
}

void EntityMesh::render(Camera * camera) {
	
	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;
	Vector3 pos = this->getPosition();

	if (!camera->testSphereInFrustum(pos, mesh->header.radius) && this->name != "stuck" ) return;

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", texture);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	if (this->name == "selectionstate_entity") {
		return;
	}
			
	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->render(camera);
	}
}

void EntityMesh::update( float elapsed_time ) {

	
}

// *************************************************************************
// ENTITYPLAYER 
// *************************************************************************

EntityPlayer::EntityPlayer() {
	mesh = NULL;
	parent = NULL;
	texture = NULL;
	shader = NULL;
}
EntityPlayer::~EntityPlayer() {}

//meshfile sin path, texturefile con path
void EntityPlayer::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = Mesh::Get(meshf);
	texture = Texture::Get(texturef);

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	
	shader = Shader::Load(vs.c_str(), fs.c_str());
	assert(shader);
}

void EntityPlayer::render(Camera * camera) {

	Matrix44 m = this->getGlobalMatrix();
	Matrix44 mvp = m * camera->viewprojection_matrix;

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", texture);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	for (int i = 0; i < this->children.size(); i++) {
		this->children[i]->render(camera);
	}
}

void EntityPlayer::update(float elapsed_time) {
	BulletManager* bManager = BulletManager::getInstance();
	World* world = World::getInstance();

	// colisiona alguna bala con los enemigos?
	for (int i = 0; i < bManager->bullet_vector.size(); i++) {

		if (bManager->bullet_vector[i].free) continue;

		for (int j = 0; j < world->collision_enemies.size(); j++) {

			EntityEnemy * current_enemy = world->collision_enemies[j];

			//si queremos especificar la model de la mesh usamos setTransform

			CollisionModel3D * collisionModel = current_enemy->mesh->getCollisionModel();

			collisionModel->setTransform(current_enemy->model.m);

			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = bManager->bullet_vector[i].last_position - bManager->bullet_vector[i].position;

			if (!collisionModel->rayCollision(bManager->bullet_vector[i].last_position.v,
				front.v, false)) continue;
			
			// collision made
			bManager->bullet_vector[i].free = true; // liberar espacio de la bala
			current_enemy->life -= bManager->bullet_vector[i].damage;
			current_enemy->life = max(current_enemy->life, 0);

			if (current_enemy->life == 0) world->root->removeChild(current_enemy);
		}
	}

	bManager->update(elapsed_time);
	world->t1->update(elapsed_time);
	world->t2->update(elapsed_time);
}

void EntityPlayer::m60Shoot() {
	BulletManager* bManager = BulletManager::getInstance();
	Game*game = Game::getInstance();
	int planeModel = World::getInstance()->worldInfo.playerModel;
	
	Vector3 cannon_pos1;
	Vector3 cannon_pos2;
	Vector3 cannon_pos3;
	switch (planeModel) {
	case 0:
		cannon_pos1 = Vector3(1.9f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-1.9f, -0.25f, 5.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		break;
	case 1:
		cannon_pos1 = Vector3(0.5f, -0.25f, 10.f);
		cannon_pos2 = Vector3(-0.5f, -0.25f, 10.f);
		cannon_pos3 = Vector3(0.f, -0.1f, 10.f);
		
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		bManager->createBullet(model*cannon_pos3, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		break;
	case 2:
		cannon_pos1 = Vector3(0.f, -0.50f, 10.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		break;
	case 3:
		cannon_pos1 = Vector3(2.40f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-2.55f, -0.25f, 5.f);
		bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
		break;
	}

	int sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it
}

void EntityPlayer::torpedoShoot() {
	
	if (!torpedosLeft) return;

	torpedosLeft--;

	int sample = BASS_SampleLoad(false, "data/sounds/missil.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it
}

// *************************************************************************
// ENTITYENEMY
// *************************************************************************

EntityEnemy::EntityEnemy() {
	mesh = NULL;
	parent = NULL;
	texture = NULL;
	shader = NULL;
}
EntityEnemy::~EntityEnemy() {}

//meshfile sin path, texturefile con path
void EntityEnemy::set(const char * meshf, const char * texturef, const char * shaderf) {

	mesh = Mesh::Get(meshf);
	texture = Texture::Get(texturef);

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	
	shader = Shader::Load(vs.c_str(), fs.c_str());
	assert(shader);
}

void EntityEnemy::render(Camera * camera) {

	Matrix44 mvp = model * camera->viewprojection_matrix;

	shader->enable();
	shader->setMatrix44("u_model", model);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", texture);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

}

void EntityEnemy::update(float elapsed_time) {
	
}

// *************************************************************************
// TORPEDO
// *************************************************************************

unsigned int Torpedo::last_tid = 0;

Torpedo::Torpedo() {
	parent = NULL;
	mesh = Mesh::Get("torpedo.ASE");
	texture = Texture::Get("data/textures/torpedo.tga");
	std::string fs = "data/shaders/simple.fs";
	std::string vs = "data/shaders/simple.vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
	assert(shader);

	name = "stuck";
	tid = last_tid;
	last_tid++;
	
	ttl = 2;
}

Torpedo::~Torpedo() {}

void Torpedo::update(float elapsed_time) {

	if (this->ttl < 0) destroy();

	EntityPlayer* player = World::getInstance()->playerAir;
	if (player->torpedosLeft <= this->tid) {
		this->model.traslate(0, 0, elapsed_time * 100);
		ttl -= elapsed_time;
	}
}


// **************************************************************************************