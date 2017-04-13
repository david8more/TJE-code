#ifndef BULLETMANAGER_H
#define BULLETMANAGER_H
#include <vector>
#include "utils.h"
#include "bullet.h"

class BulletManager {

public:

	static BulletManager* instance;

	static BulletManager* getInstance() {
		return instance;
	}

	BulletManager();
	~BulletManager();

	std::vector<Bullet> bullet_vector;
	int last_free;

	void createBullet(Vector3 position, Vector3 velocity, float ttl, float damage, int author, int type);

	void render();
	void update( float elapsed_time );

};

#endif // !BULLETMANAGER_H
