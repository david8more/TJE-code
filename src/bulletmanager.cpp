#include "utils.h"
#include <iostream>
#include <vector>
#include "bulletmanager.h"
#include "bullet.h"
#include "mesh.h"
#include "game.h"
#include "framework.h"

BulletManager* BulletManager::instance = NULL;

BulletManager::BulletManager() {
	bullet_vector.resize(200);
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
			bullets.colors.push_back(Vector4(1, 0, 0, 1));
			break;
		case 2:
			bullets.colors.push_back(Vector4(0, 1, 0, 1));
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

	glEnable(GL_BLEND);
	bullets.render(GL_LINES);
	glDisable(GL_BLEND);
}

void BulletManager::update(float elapsed_time) {
	
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
}