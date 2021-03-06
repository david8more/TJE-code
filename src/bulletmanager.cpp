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
#include "explosion.h"

BulletManager* BulletManager::instance = NULL;

BulletManager::BulletManager()
{
	bullet_vector.resize(500);
	last_free = 0;
	instance = this;
}

BulletManager::~BulletManager()
{
	
}

void BulletManager::createBullet(Vector3 position, Vector3 velocity, float ttl, float damage, Entity* author, int type)
{
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
		
		Vector4 color(1.f, 1.f, 1.f, 1.f);

		if(current.type == 2)
			color = Vector4(1.f, 0.f, 1.f, 1.f);

		bullets.colors.push_back(color);
		bullets.colors.push_back(Vector4(1.f, 1.f, 1.f, 0.0));
	}

	if (!bullets.vertices.size())
		return;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(2.0);
	bullets.render(GL_LINES);
	glDisable(GL_BLEND);
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
		current.velocity += Vector3(0.f, -28.f * elapsed_time, 0.f);
		// friccion aire
		current.velocity = current.velocity * 0.99;
	}

	//if(last_free > 0)
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

			if (current_enemy == current.author || (current.author->uid > 1000 && current_enemy->uid > 1000) )
				continue;

			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision m�s cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = current.last_position - current.position;

			if (collisionModel->rayCollision(current.last_position.v, front.v, true))
			{
				// collision made
				Vector3 collisionPoint(0, 0, 0);
				collisionModel->getCollisionPoint(collisionPoint.v, false);
				current_enemy->onBulletCollision(collisionPoint, current);
				current.ttl = -1.f;
			}
			
		}
	}

}