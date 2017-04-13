#include "state.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "entity.h"
#include "shader.h"
#include "bulletmanager.h"

// ENTITY

Entity::Entity() {
	parent = NULL;
}

Entity::~Entity() {}
void Entity::removeChild(Entity* entity) {}

Vector3 Entity::getPosition() { 
	return model * Vector3();
}

void Entity::render() {
	for (int i = 0; i < children.size(); i++) {
		children[i]->render();
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

	mesh = new Mesh();

	if (!mesh->loadASE(meshf))
		exit(0);
	mesh->uploadToVRAM();

	texture = new Texture();
	if (texture->load(texturef))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error: texture has not been loaded" << endl;
		exit(1);
	}
	shader = new Shader();
	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	if (!shader->load(vs, fs))
	{
		std::cout << "shader not found or error" << std::endl;
		exit(0);
	}

	
}

void EntityMesh::render() {
	
	Game* game = Game::getInstance();
	Matrix44 mvp = model * game->camera->viewprojection_matrix;

	shader->enable();
	shader->setMatrix44("u_model", model);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", texture);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();
			
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

	mesh = new Mesh();

	if (!mesh->loadASE(meshf))
		exit(0);
	mesh->uploadToVRAM();

	texture = new Texture();
	if (texture->load(texturef))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error: texture has not been loaded" << endl;
		exit(1);
	}
	shader = new Shader();
	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";
	if (!shader->load(vs, fs))
	{
		std::cout << "shader not found or error" << std::endl;
		exit(0);
	}

}

void EntityPlayer::render() {

	Game* game = Game::getInstance();
	Matrix44 mvp = model * game->camera->viewprojection_matrix;

	shader->enable();
	shader->setMatrix44("u_model", model);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", texture);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

}

void EntityPlayer::update(float elapsed_time) {
	BulletManager* bManager = BulletManager::getInstance();

	bManager->update(elapsed_time);
}

void EntityPlayer::m60Shoot() {
	BulletManager* bManager = BulletManager::getInstance();

	Game* game = Game::getInstance();
	bManager->createBullet(model*Vector3(1.85, -0.25, 10), model.rotateVector(Vector3(-15, 0, 1000)), 1, 10.0, 0, 1);
	bManager->createBullet(model*Vector3(-2, -0.25, 10), model.rotateVector(Vector3(-15, 0, 1000)), 1, 10.0, 0, 2);
}

void EntityPlayer::missileShoot() {
	BulletManager* bManager = BulletManager::getInstance();

	Game* game = Game::getInstance();
	bManager->createBullet(model*Vector3(0, -0.50, 10), model.rotateVector(Vector3(-15, 0, 1000)), 1, 100.0, 0, 3);
}