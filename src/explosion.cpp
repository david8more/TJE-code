#include "explosion.h"
#include "mesh.h"
#include "entity.h"
#include "texture.h"
#include "framework.h"

std::vector<Explosion*> Explosion::v_explosions;
std::vector<Explosion*> Explosion::destroyed_exp;

Explosion::Explosion() : time(0.0)
{ }

Explosion::~Explosion()
{ }

void Explosion::createExplosion(Vector3 atCollision, float size, float ttl)
{
    Explosion * newExplosion = new Explosion();
	newExplosion->collision = atCollision;
	newExplosion->ttl = ttl;
	newExplosion->size = size;
    Explosion::v_explosions.push_back(newExplosion);
}

void Explosion::render(Camera * camera)
{
	if(!v_explosions.size())
        return;

    Mesh m;

	for (int i = 0; i < v_explosions.size(); i++)
	{
		Explosion * current = v_explosions[i];

        Vector3 c = current->collision;
		Vector3 up = camera->up;
		Vector3 right = (camera->center - camera->eye).cross(up);

        up.normalize();
        right.normalize();

		float size = current->size;

		m.vertices.push_back(c - right * size * 0.5 + up * size * 0.5);
		m.vertices.push_back(c + right * size * 0.5 + up * size * 0.5);
		m.vertices.push_back(c - right * size * 0.5 - up * size * 0.5);

		m.vertices.push_back(c + right * size * 0.5 + up * size * 0.5);
		m.vertices.push_back(c + right * size * 0.5 - up * size * 0.5);
		m.vertices.push_back(c - right * size * 0.5 - up * size * 0.5);

		// coordenadas textura 0..1
		// de un fuego a otro van 1/5
		float step =  1 / 5.f;

		float f = current->time / current->ttl;
		unsigned frame = f * 25.0;

		int x = frame % 5;
		int y = frame / 5.0;

		m.uvs.push_back(Vector2(x * step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step, y * step));
		
		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step));
		m.uvs.push_back(Vector2(x * step, y * step));
    }

	Texture * texture = Texture::Get("data/textures/explosion.tga");

	glColor4f(1, 1, 1, 1.0);

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE); // additive blending

	texture->bind();
    m.render(GL_TRIANGLES);
	texture->unbind();

    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

}

void Explosion::update(float elapsed_time)
{
    if(!v_explosions.size())
        return;

	for (int i = 0; i < v_explosions.size(); i++)
	{
		v_explosions[i]->time += elapsed_time;
    }

	Explosion* first = *v_explosions.begin();

    if(first->time >= first->ttl)
	{
		v_explosions.erase(v_explosions.begin());
    }
}