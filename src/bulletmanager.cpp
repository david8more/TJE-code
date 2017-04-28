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
	bullet_vector.resize(30);
	last_free = 0;


	for (int i = 0; i < bullet_vector.size(); i++) {
		bullet_vector[i].free = true;
	}

	instance = this;
}

BulletManager::~BulletManager() {
	
}

void BulletManager::createBullet(Vector3 position, Vector3 velocity, float ttl, float damage, int author, int type) {
	for (int i = 0; i < bullet_vector.size(); i++) {

		if (!bullet_vector[i].free) {
			continue;
		}

		bullet_vector[i].position = position;
		bullet_vector[i].last_position = position;
		bullet_vector[i].velocity = velocity;
		bullet_vector[i].ttl = ttl;
		bullet_vector[i].damage = damage;
		bullet_vector[i].author = author;
		bullet_vector[i].type = type;
		bullet_vector[i].free = false;
		return;
	}
}

void BulletManager::render() {
	Mesh bullets;

	for (int i = 0; i < bullet_vector.size(); ++i) {
		if (bullet_vector[i].free) continue;

		bullets.vertices.push_back(bullet_vector[i].position);
		bullets.vertices.push_back(bullet_vector[i].last_position);

		switch (bullet_vector[i].type) {
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
	
	World* world = World::getInstance();

	for (int i = 0; i < bullet_vector.size(); i++) {

		bullet_vector[i].ttl -= elapsed_time;

		if (bullet_vector[i].ttl < 0.0) {
			bullet_vector[i].free = true;
			last_free = i;
			continue;
		}
		bullet_vector[i].last_position = bullet_vector[i].position;
		bullet_vector[i].position.x += bullet_vector[i].velocity.x * elapsed_time / 2;
		bullet_vector[i].position.y += bullet_vector[i].velocity.y * elapsed_time / 2;
		bullet_vector[i].position.z += bullet_vector[i].velocity.z * elapsed_time / 2;
	}

	// colisiona alguna bala con los enemigos?
	for (int i = 0; i < this->bullet_vector.size(); i++) {

		if (this->bullet_vector[i].free) continue;

		for (int j = 0; j < world->collision_enemies.size(); j++) {

			EntityEnemy * current_enemy = world->collision_enemies[j];

			//si queremos especificar la model de la mesh usamos setTransform

			Mesh* enemy_mesh = Mesh::Get(current_enemy->mesh.c_str());
			CollisionModel3D * collisionModel = enemy_mesh->getCollisionModel();

			collisionModel->setTransform(current_enemy->model.m);

			//testeamos la colision, devuelve false si no ha colisionado, es importante recordar
			//que el tercer valor sirve para determinar si queremos saber la colision más cercana 
			//al origen del rayo o nos conformamos con saber si colisiona. 

			Vector3 front = this->bullet_vector[i].last_position - this->bullet_vector[i].position;

			if (!collisionModel->rayCollision(this->bullet_vector[i].last_position.v,
				front.v, false)) continue;

			// collision made
			this->bullet_vector[i].free = true; // liberar espacio de la bala
			current_enemy->life -= this->bullet_vector[i].damage;
			current_enemy->life = max(current_enemy->life, 0);

			if (current_enemy->life == 0) world->root->removeChild(current_enemy);
		}
	}

}