#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <vector>
#include "framework.h"
#include "camera.h"

class Explosion {
public:
    Explosion(Vector3 collisionPoint);
    ~Explosion();
 
    Vector3 collision;
    float time;
    float lifetime;
	
	static std::vector<Explosion*> v_explosions;

    static void render(Camera* camera);
    static void update(float elapsed_time);
    static void createExplosion(Vector3 atCollision);

};


#endif 
