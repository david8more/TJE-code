#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>
#include "framework.h"
#include "camera.h"
#include "entity.h"

class Explosion : public EntityMesh {
public:
    Explosion();
    ~Explosion();
 
    Vector3 collision;
    float time;
    float ttl;
	float size;
	
	static std::vector<Explosion*> v_explosions;

	static void render(Camera* camera);
    static void update(float elapsed_time);
    static void createExplosion(Vector3 atCollision, float size = 15.0, float ttl = 3.5);

};


#endif 
