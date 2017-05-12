#include "utils.h"
#include <iostream>
#include <vector>
#include "bulletmanager.h"
#include "bullet.h"
#include "mesh.h"
#include "game.h"
#include "world.h"
#include "framework.h"
#include "entity.h"

BulletManager* BulletManager::instance = NULL;

BulletManager::BulletManager() {
	bullet_vector.resize(100);
	last_free = 0;
	instance = this;
}

BulletManager::~BulletManager() {
	
}

void BulletManager::createBullet(Vector3 position, Vector3 velocity, float ttl, float damage, Entity* author, int type) {

	if (last_free == bullet_vector.size())
		return;

	Bullet& current = bullet_vector[last_free];

	current.position = position;
	current.last_position = position;
	current.velocity = velocity;
	current.ttl = ttl;
	current.damage = damage;
	current.author = author;
	current.type = type;

	last_free++;
}

void BulletManager::render()
{	
	if (last_free == 0)
		return;

	Mesh bullets;

	for (int i = 0; i < last_free; i++)
	{
		Bullet& current = bullet_vector[i];
		
		bullets.vertices.push_back(current.position);
		bullets.vertices.push_back(current.last_position);
		
		bullets.colors.push_back(Vector4(1.f, 0.f, 0.f, 1.f));
		bullets.colors.push_back(Vector4(1, 1, 0, 1));
	}

	if (!bullets.vertices.size())
		return;

	glLineWidth(1.5);
	bullets.render(GL_LINES);
}

void BulletManager::update(float elapsed_time)
{	
	for (int i = 0; i < last_free; i++)
	{
		Bullet& current = bullet_vector[i];
		current.ttl -= elapsed_time;

		if (current.ttl <= 0.0)
		{	
			if (last_free)
			{
				current = bullet_vector[last_free - 1];
				last_free--;
				i--;
			}
			continue;
		}
		current.last_position = current.position;
		current.position += current.velocity * elapsed_time;
		current.velocity += Vector3(0.f, -50.f * elapsed_time, 0.f);
		// friccion aire
		current.velocity = current.velocity * 0.99;
	}

	testBulletCollision();

}

void BulletManager::testBulletCollision() {

	// colisiona alguna bala con los enemigos?
	for (int i = 0; i < EntityCollider::dynamic_colliders.size(); i++) {

		EntityCollider * current_enemy = EntityCollider::dynamic_colliders[i];

		//si queremos especificar la model de la mesh usamos setTransform

		Mesh* enemy_mesh = Mesh::Get(current_enemy->mesh.c_str());
		CollisionModel3D * collisionModel = enemy_mesh->getCollisionModel();

		collisionModel->setTransform(current_enemy->model.m);

		for (int j = 0; j < last_free; j++) {

			Bullet& current = bullet_vector[j];

			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = current.last_position - current.position;

			if (collisionModel->rayCollision(current.last_position.v, front.v, false))
			{
				// collision made
				current_enemy->onBulletCollision();
				current.ttl = -1.f;
			}
			
		}
	}

}