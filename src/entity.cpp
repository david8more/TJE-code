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
#include "soundmanager.h"
#include "explosion.h"
#include "world.h"
#include <cassert>
#include <algorithm>

// ENTITY

std::vector<Entity*> Entity::destroy_pending;
std::map<std::string, Entity*> Entity::s_Entities;
std::map<unsigned int, Entity*> Entity::s_EntitiesUID;

Entity::Entity() {
	parent = NULL;
	destroyed = false;
}

Entity::~Entity()
{
	auto it = s_Entities.find(name);
	if (it != s_Entities.end())
		s_Entities[name] = NULL;

	EntityCollider::remove(this); // quitarse del vector de static y dinamics SI ESTOY
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
	else return NULL;
}

void Entity::setUid(unsigned int uid)
{
	this->uid = uid;
	s_EntitiesUID[uid] = this;
}

Entity* Entity::getEntity(unsigned int uid)
{
	auto it = s_EntitiesUID.find(uid);
	if (it != s_EntitiesUID.end())
		return it->second;
	else return NULL;
}

Vector3 Entity::getPosition() { 
	return model * Vector3();
}

void Entity::render(Camera * camera)
{
	for (int i = 0; i < children.size(); i++)
		children[i]->render(camera);
}

void Entity::update(float elapsed_time)
{
	for (int i = 0; i < children.size(); i++)
		children[i]->update(elapsed_time);
}

void Entity::addChild(Entity * entity)
{
	if (entity->parent) {
		return;
	}
	entity->parent = this;
	children.push_back( entity );
}

void Entity::removeChild(Entity* entity)
{
	if (!children.size())
		return;

	auto it = std::find(children.begin(), children.end(), entity);
	if (it == children.end())
		return;

	children.erase(it);
	entity->parent = NULL; // tu padre ya no soy yo
	entity->model = entity->model * this->getGlobalMatrix(); // tu posicion ahora es de mundo

	// quitar hijos del hijo
	for (int i = 0; i < entity->children.size(); i++)
	{
		Entity* son = entity->children[i];

		auto it = std::find(children.begin(), children.end(), son);
		if (it == children.end())
			continue;

		entity->children.erase(it);
		son->parent = NULL; // tu padre ya no soy yo
	}
}

Matrix44 Entity::getGlobalMatrix()
{
	if( parent ) return model * parent->getGlobalMatrix();
	else return model;
}

void Entity::destroy()
{
	if (destroyed)
		return;
	
	destroy_pending.push_back(this);
	destroyed = true;
	
	for (int i = 0; i < children.size(); i++)
	{
		children[i]->destroy();
	}
}

void Entity::destroy_entities()
{
	for (int i = 0; i < destroy_pending.size(); i++)
	{
		Entity* ent = destroy_pending[i];
		//std::cout << "Destroying " << ent->name << std::endl;
		if(ent->parent != NULL)
		   ent->parent->removeChild(ent); // desvincular hijo del padre
		delete(ent);
	}

	destroy_pending.clear();
}

// *************************************************************************
// ENTITYMESH 
// *************************************************************************

EntityMesh::EntityMesh(bool frust_culling)
{
	culling = frust_culling;
	cullFace = true;
	alpha = false;
	depthTest = true;
	depthMask = true;
}
EntityMesh::~EntityMesh() {}

//meshfile sin path, texturefile con path
void EntityMesh::set(const char * meshf, const char * texturef, const char * shaderf )
{
	mesh = meshf;
	texture = texturef;

	std::string shader_string(shaderf);
	std::string fs = "data/shaders/" + shader_string + ".fs";
	std::string vs = "data/shaders/" + shader_string + ".vs";

	shader = Shader::Load(vs.c_str(), fs.c_str());
}

void EntityMesh::render(Camera * camera)
{
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

void EntityCollider::setLife(int life)
{
	this->life = life;
	max_life = life;
}

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

		if (!collisionModel->rayCollision(origin.v, dir.v, 0.0, max_dist, true))
			continue;

		collisionModel->getCollisionPoint(collisions.v, false);
		return true;
	}

	return false;
}

void EntityCollider::testSphereCollision()
{
	// concatenate two vectors
	/*std::vector<EntityCollider*> all;
	all.reserve(EntityCollider::static_colliders.size() + EntityCollider::dynamic_colliders.size()); // preallocate memory
	all.insert(all.end(), EntityCollider::static_colliders.begin(), EntityCollider::static_colliders.end());
	all.insert(all.end(), EntityCollider::dynamic_colliders.begin(), EntityCollider::dynamic_colliders.end());
	*/
	for (int i = 0; i < EntityCollider::dynamic_colliders.size(); i++)
	{
		EntityCollider* current = EntityCollider::dynamic_colliders[i];

		// no queremos hacer las colisiones jugador/barcos, se hará con el rayo
		// -> tiene más precision
		if (current == this || current->uid == this->uid)
			continue;

		//std::cout << this->name << ": " << current->name << endl;

		Mesh * my_mesh = Mesh::Get(mesh.c_str());
		Mesh * enemy_mesh = Mesh::Get(current->mesh.c_str());

		Vector3 my_position = model * my_mesh->header.center;
		Vector3 enemy_position = current->model * enemy_mesh->header.center;

		float margin = current->name == "ship" ? 15.0 : 3.0;
		float dist = my_position.distance(enemy_position) + margin;

		if (dist < (my_mesh->header.radius + enemy_mesh->header.radius))
		{
			//std::cout << "Collided with:" << current->name << std::endl;
			this->onCollision(current);
		}
	}

}

void EntityCollider::testStaticCollisions()
{
	if (!Game::getInstance()->start)
		return;

	if (this->getPosition().y < -7.5)
	{
		std::cout << "Water collision" << std::endl;
		onCollision(NULL);
		return;
	}

	// get current position after updating
	Vector3 current_position = getPosition();

	Vector3 ray_origin;
	Vector3 ray_dir;

	ray_origin = last_position;

	if (current_position == last_position)
		return;

	ray_dir = (current_position - ray_origin).normalize();

	Vector3 coll;
	int maxT = (current_position - ray_origin).length();

	if (EntityCollider::testRayWithAll(ray_origin, ray_dir, maxT, coll)) {
		onCollision(NULL);
	}
}

void EntityCollider::remove(Entity* ent)
{
	//
	std::vector<EntityCollider*>::iterator it;
	it = std::find(static_colliders.begin(), static_colliders.end(), ent);

 	if (it != static_colliders.end())
		static_colliders.erase(it);

	it = std::find(dynamic_colliders.begin(), dynamic_colliders.end(), ent);

	if (it != dynamic_colliders.end())
		dynamic_colliders.erase(it);
	//
}

void EntityCollider::onBulletCollision(Vector3 collisionPoint, Bullet& b)
{
	this->life -= b.damage;
	this->life = max(this->life, 0);

	Explosion::createExplosion(collisionPoint);

	if (!this->life) {
		SoundManager::getInstance()->playSound("explosion", false);
		unboundController();
		destroy();
	}
}

