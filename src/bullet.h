#ifndef BULLET_H
#define BULLET_H
#include "utils.h"

class Bullet {

public:
	Vector3 position;
	Vector3 last_position;
	Vector3 velocity;
	float ttl; // lifetime
	float damage;
	int author; // who shot
	int type; // bullet type
	bool free;
};

#endif