#ifndef BULLET_H
#define BULLET_H
#include "utils.h"
#include "entity.h"

class Entity;

class Bullet {

public:

	Vector3 position;
	Vector3 last_position;
	Vector3 velocity;
	float ttl; // lifetime
	float damage;
	Entity* author; // who shot
	int type; // bullet type
	bool free;
};

#endif