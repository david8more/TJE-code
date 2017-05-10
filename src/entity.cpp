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
std::map<std::string, Entity*> Entity::s_Entities;

Entity::Entity() {
	parent = NULL;
}

Entity::~Entity()
{
	auto it = s_Entities.find(name);
	if (it != s_Entities.end())
		s_Entities[name] = NULL;
}

void Entity::setName(std::string name)
{
	this->name = name;
	s_Entities[name] = this;
}

Entity* Entity::getEntity(std::string name)
{
	auto it = s_Entities.find(name);
	if (it != s_Entities.end())
		return it->second;
}

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
		else
		{
			children[i]->render(camera);
		}
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
	if (it == children.end())
		return;

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

	for (int i = 0; i < destroy_pending.size(); i++)
	{
		Entity* ent = destroy_pending[i];
  		EntityCollider::removeStatic(ent);
		ent->parent->removeChild(ent);
		ent->parent = NULL;
		delete(ent);

	}

	destroy_pending.clear();
}

// *************************************************************************
// ENTITYMESH 
// *************************************************************************

EntityMesh::EntityMesh(bool frust_culling) {
	culling = frust_culling;
	cullFace = true;
	alpha = false;
	depthTest = true;
	depthMask = true;
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

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", mvp);
	shader->setTexture("u_texture", Texture::Get(this->texture.c_str()));
	shader->setTexture("u_sky_texture", Texture::Get(Game::getInstance()->gameMode ? "data/textures/cielo-heroic.tga" : "data/textures/cielo.tga"));
	shader->setFloat("u_time", Game::getInstance()->time);
	shader->setVector3("u_camera_pos", Game::getInstance()->current_camera->eye);
	mesh->render(GL_TRIANGLES, shader);
	shader->disable();

	if (!depthTest)
		glEnable(GL_DEPTH_TEST);
	if(!depthMask)
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

void EntityMesh::update( float elapsed_time )
{
	
}

// *************************************************************************
// ENTITYCOLLIDER
// *************************************************************************

EntityCollider::EntityCollider()
{
	is_static = false;
	is_dynamic = false;
}

EntityCollider::~EntityCollider() {}

std::vector<EntityCollider*> EntityCollider::static_colliders;
std::vector<EntityCollider*> EntityCollider::dynamic_colliders;

void EntityCollider::setStatic()
{
	static_colliders.push_back(this);
	is_static = true;
}

void EntityCollider::setDynamic()
{
	dynamic_colliders.push_back(this);
	is_dynamic = true;
}

bool EntityCollider::testRayWithAll(Vector3 origin, Vector3 dir, float max_dist, Vector3& collisions)
{
	for (int j = 0; j < static_colliders.size(); j++) {

		EntityCollider * current_enemy = static_colliders[j];

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

void EntityCollider::testSphereCollision()
{
	for (int i = 0; i < EntityCollider::static_colliders.size(); i++)
	{
		EntityCollider* current = EntityCollider::static_colliders[i];

		Mesh * my_mesh = Mesh::Get(mesh.c_str());
		Mesh * enemy_mesh = Mesh::Get(current->mesh.c_str());

		Vector3 my_position = model * my_mesh->header.center;
		Vector3 enemy_position = current->model * enemy_mesh->header.center;

		float margin = current->name == "enemy_ship" ? 25.0 : 5.0;
		float dist = my_position.distance(enemy_position) + margin;

		if (dist < (my_mesh->header.radius + enemy_mesh->header.radius))
		{
			onCollision(current);
		}

	}
}

void EntityCollider::removeStatic(Entity* ent)
{
	std::vector<EntityCollider*>::iterator it;

	it = std::find(static_colliders.begin(), static_colliders.end(), ent);

 	if (it != static_colliders.end())
		static_colliders.erase(it);
}

void EntityCollider::onBulletCollision()
{
	this->life -= 5;
	this->life = max(this->life, 0);

	if (!this->life) {
		destroy();
	}
}