#include "states/state.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "meshmanager.h"
#include "texture.h"
#include "entity.h"
#include "shader.h"
#include "bulletmanager.h"
#include "world.h"
#include "bass.h"
#include <cassert>
#include <algorithm>

// ENTITY

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

	mesh = MeshManager::getInstance()->getMesh(meshf, false);
	mesh->uploadToVRAM();

	texture = Texture::Get(texturef);

	shader = new Shader();
	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
	assert(shader);
}

void EntityMesh::render(Camera * camera) {
	
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

	mesh = MeshManager::getInstance()->getMesh(meshf, false);
	mesh->uploadToVRAM();

	texture = Texture::Get(texturef);

	shader = new Shader();
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

			//si queremos especificar la model de la mesh usamos setTransform
			world->collision_enemies[j]->mesh->collision_model->setTransform(world->collision_enemies[j]->model.m);
			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = bManager->bullet_vector[i].last_position - bManager->bullet_vector[i].position;

			if (!world->collision_enemies[j]->mesh->collision_model->rayCollision(bManager->bullet_vector[i].last_position.v,
				front.v, false)) continue;
			
			// collision made
			bManager->bullet_vector[i].free = true; // liberar espacio de la bala
			if (bManager->bullet_vector[i].type == 3) {
				int sample = BASS_SampleLoad(false, "data/sounds/explosion.wav", 0L, 0, 1, 0);
				int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
				BASS_ChannelPlay(channel, false); // play it
			}
			world->collision_enemies[j]->life -= bManager->bullet_vector[i].damage;
			world->collision_enemies[j]->life = max(world->collision_enemies[j]->life, 0);
			if (world->collision_enemies[j]->life == 0) {
				world->collision_enemies[j]->visible = false;
				world->collision_enemies[j]->model.setTranslation(0.f, -15000.f, 0.f);
			}	
		}
	}

	bManager->update(elapsed_time);
}

void EntityPlayer::m60Shoot() {
	BulletManager* bManager = BulletManager::getInstance();
	int planeModel = World::getInstance()->worldInfo.playerModel;
	
	Vector3 cannon_pos1;
	Vector3 cannon_pos2;

	switch (planeModel) {
	case 0:
		cannon_pos1 = Vector3(1.85f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-2.f, -0.25f, 5.f);
		break;
	case 1:
		cannon_pos1 = Vector3(2.60f, -0.25f, 10.f);
		cannon_pos2 = Vector3(-2.75f, -0.25f, 10.f);
		break;
	case 2:
		cannon_pos1 = Vector3(0.f, -0.50f, 10.f);
		cannon_pos2 = Vector3(0.f, 0.f, -99.f); // -99.f implica que no hay
		break;
	case 3:
		cannon_pos1 = Vector3(2.40f, -0.25f, 5.f);
		cannon_pos2 = Vector3(-2.55f, -0.25f, 5.f);
		break;
	}

	bManager->createBullet(model*cannon_pos1, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);
	if(cannon_pos2.z != -99.f) bManager->createBullet(model*cannon_pos2, model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageM60, 0, 1);

	int sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
	int channel = BASS_SampleGetChannel(sample, false); // get a sample channel
	BASS_ChannelPlay(channel, false); // play it
}

void EntityPlayer::missileShoot() {
	BulletManager* bManager = BulletManager::getInstance();
	
	bManager->createBullet(model*Vector3(0, -0.50, 10), model.rotateVector(Vector3(0.f, 0.f, 1000.f)), 1, this->damageMissile, 0, 3);
	this->missilesLeft--;

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

	//mesh = MeshManager::getInstance()->getMesh(meshf, true);
	mesh = new Mesh();
	if (!mesh->loadASE(meshf, true)) {
		exit(0);
	}
	mesh->uploadToVRAM();

	texture = Texture::Get(texturef);

	shader = new Shader();
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

/*void EntityEnemy::m60Shoot() {
	
}

void EntityEnemy::missileShoot() {
	
}*/