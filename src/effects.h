#ifndef FLASH_H
#define FLASH_H

#include <vector>
#include "framework.h"
#include "camera.h"
#include "entity.h"

class Flash : public EntityMesh {
public:
	Flash();
    ~Flash();
 
    Vector3 position;
	Entity* author;
    float time;
    float ttl;
	float size;
	
	static std::vector<Flash*> v_flashes;

	static void render(Camera* camera);
    static void update(float elapsed_time);
    static void createFlash(Vector3 position, Entity* author, float size = 1.5, float ttl = 0.1);

};

class Smoke : public EntityMesh {
public:
	Smoke();
	~Smoke();

	Vector3 position;
	Entity* author;
	float time;
	float ttl;
	float size;

	static std::vector<Smoke*> v_smoke;

	static void render(Camera* camera);
	static void update(float elapsed_time);
	static void createSmoke(Vector3 position, Entity* author, float size = 1.5, float ttl = 0.2);

};


#endif 
