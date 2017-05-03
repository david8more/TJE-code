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

	if (last_free >= bullet_vector.size())
		return;

	Bullet& current = bullet_vector[last_free];

	current.position = position;
	current.last_position = position;
	current.velocity = velocity;
	current.ttl = ttl;
	current.damage = damage;
	current.author = author;
	current.type = type;
	current.free = false;

	last_free++;
}

void BulletManager::render() {
	Mesh bullets;
	std::cout << last_free << std::endl;

	for (int i = 0; i < bullet_vector.size(); i++) {

		Bullet& current = bullet_vector[i];

		if (current.ttl < 0.0)
			continue;

		bullets.vertices.push_back(current.position);
		bullets.vertices.push_back(current.last_position);

		switch (current.type) {
		case 1:
			bullets.colors.push_back(Vector4(0.f, 0.f, 0.f, 1.f));
			break;
		case 2:
			bullets.colors.push_back(Vector4(1.f, 0.f, 0.f, 1.f));
			break;
		case 3:
			bullets.colors.push_back(Vector4(1, 0, 1, 1));
			break;
		default:
			break;
		}

		bullets.colors.push_back(Vector4(1, 1, 0, 1));
	}

	if (!bullets.vertices.size()) return;

	glLineWidth(2);
	glEnable(GL_BLEND);
	bullets.render(GL_LINES);
	glDisable(GL_BLEND);
}

void BulletManager::update(float elapsed_time) {
	
	for (int i = 0; i < last_free; i++) {

		Bullet& current = bullet_vector[i];

		current.ttl -= elapsed_time;

		if (current.ttl < 0.0) {
			current = bullet_vector[last_free - 1];
			last_free--;
			//i = i - 1;
			continue;
		}

		current.last_position = current.position;
		current.position.x += current.velocity.x * elapsed_time / 2;
		current.position.y += current.velocity.y * elapsed_time / 2;
		current.position.z += current.velocity.z * elapsed_time / 2;
	}

	if (testBulletCollision())
		onBulletCollision();

}

bool BulletManager::testBulletCollision() {

	// colisiona alguna bala con los enemigos?
	for (int i = 0; i < EntityCollider::static_colliders.size(); i++) {

		EntityCollider * current_enemy = EntityCollider::static_colliders[i];

		//si queremos especificar la model de la mesh usamos setTransform

		Mesh* enemy_mesh = Mesh::Get(current_enemy->mesh.c_str());
		CollisionModel3D * collisionModel = enemy_mesh->getCollisionModel();

		collisionModel->setTransform(current_enemy->model.m);

		for (int j = 0; j < this->bullet_vector.size(); j++) {

			Bullet current = this->bullet_vector[j];

			if (current.ttl < 0.0)
				continue;

			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = current.last_position - current.position;

			if (!collisionModel->rayCollision(current.last_position.v, front.v, false))
				continue;

			// collision made
			return true;
		}
	}

	return false;
}

void BulletManager::onBulletCollision() {

}