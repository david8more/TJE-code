#ifndef ENTITY_H
#define ENTITY_H

#include "framework.h"
#include "utils.h"
#include <iostream>
#include <map>

#define NO_CULLING false
#define CULLING true

using namespace std;

class Mesh;
class Texture;
class Shader;
class Torpedo;
class Camera;

class Entity {
public:	
	Entity();
	~Entity();

	Matrix44 model;
	unsigned int uid;
	string name;
	Entity* parent;
	vector<Entity*> children;

	static std::map<std::string, Entity*> s_Entities;
	static Entity* getEntity(std::string name);
	void setName(std::string name);

	virtual void render(Camera * camera);
	virtual void update(float elapsed_time);
	void removeChild(Entity* entity);
	void addChild(Entity* entity);
	Matrix44 getGlobalMatrix();
	Vector3 getPosition();

	void destroy();
	static void destroy_entities();
	static std::vector<Entity*> destroy_pending;
	
};

// *******************************************************************
// adding mesh and texture to get the visual representation

class EntityMesh: public Entity {
public:

	EntityMesh(bool culling = true);
	~EntityMesh();

	std::string mesh;
	std::string texture;
	Shader* shader;
	
	bool culling;
	bool alpha;
	bool cullFace;
	bool depthTest;
	bool depthMask;

	void set(const char * mesh, const char * texture, const char * shader);
	void render(Camera * camera);
	void update(float elapsed_time);

};
// *******************************************************************

class EntityCollider : public EntityMesh {
public:
	bool is_static;
	bool is_dynamic;

	static std::vector<EntityCollider*> dynamic_colliders;
	static std::vector<EntityCollider*> static_colliders;

	EntityCollider();
	~EntityCollider();
	virtual void onBulletCollision();
	virtual void onCollision(EntityCollider* collided_with) {}
	void testSphereCollision();
	static void removeStatic(Entity* ent);

	int life;

	void setStatic();
	void setDynamic();

	static bool testRayWithAll(Vector3 origin, Vector3 dir, float max_dist, Vector3& collisions);

};

// *******************************************************************

#endif