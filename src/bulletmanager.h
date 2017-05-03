#ifndef BULLETMANAGER_H
#define BULLETMANAGER_H
#include <vector>
#include "utils.h"
#include "entity.h"
#include "bullet.h"

class BulletManager {

private:
	BulletManager();
	~BulletManager();
public:

	static BulletManager* instance;

	static BulletManager* getInstance() {
		if (instance == NULL) instance = new BulletManager();
		return instance;
	}

	std::vector<Bullet> bullet_vector;
	int last_free;

	void createBullet(Vector3 position, Vector3 velocity, float ttl, float damage, Entity* author, int type);

	void render();
	void update( float elapsed_time );
	bool testBulletCollision();
	void onBulletCollision();
};

#endif // !BULLETMANAGER_H
