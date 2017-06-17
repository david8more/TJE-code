#include "effects.h"
#include "mesh.h"
#include "entity.h"
#include "texture.h"
#include "framework.h"

std::vector<Flash*> Flash::v_flashes;

Flash::Flash() : time(0.0)
{ }

Flash::~Flash()
{ }

void Flash::createFlash(Vector3 position, Entity* author, float size, float ttl)
{
	Flash * newFlash = new Flash();
	newFlash->position = position;
	newFlash->ttl = ttl;
	newFlash->size = size;
	newFlash->author = author;
	Flash::v_flashes.push_back(newFlash);
}

void Flash::render(Camera * camera)
{
	if(!v_flashes.size())
        return;

    Mesh m;

	for (int i = 0; i < v_flashes.size(); i++)
	{
		Flash * current = v_flashes[i];

        Vector3 c = current->author->model * current->position;
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
		// de un flash a otro van 1/2
		float step =  1 / 2.f;

		float f = current->time / current->ttl;
		unsigned frame = f * 4.0;

		int x = frame % 2;
		int y = frame / 2.0;

		m.uvs.push_back(Vector2(x * step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step, y * step));
		
		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step));
		m.uvs.push_back(Vector2(x * step, y * step));
    }

	Texture * texture = Texture::Get("data/textures/flash.tga");

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

void Flash::update(float elapsed_time)
{
    if(!v_flashes.size())
        return;

	for (int i = 0; i < v_flashes.size(); i++)
	{
		v_flashes[i]->time += elapsed_time;
    }

	Flash* first = *v_flashes.begin();

    if(first->time >= first->ttl)
	{
		v_flashes.erase(v_flashes.begin());
    }
}

// ****************
// SMOKE
// ****************

std::vector<Smoke*> Smoke::v_smoke;

Smoke::Smoke() : time(0.0)
{ }

Smoke::~Smoke()
{ }

void Smoke::createSmoke(Vector3 position, Entity* author, float size, float ttl)
{
	Smoke * newSmoke = new Smoke();
	newSmoke->position = position;
	newSmoke->ttl = ttl;
	newSmoke->size = size;
	newSmoke->author = author;
	Smoke::v_smoke.push_back(newSmoke);
}

void Smoke::render(Camera * camera)
{
	if (!v_smoke.size())
		return;

	Mesh m;

	for (int i = 0; i < v_smoke.size(); i++)
	{
		Smoke * current = v_smoke[i];

		Vector3 c = current->position;
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
		// de un flash a otro van 1/2
		float step = 1 / 2.f;

		float f = current->time / current->ttl;
		unsigned frame = f * 4.0;

		int x = frame % 2;
		int y = frame / 2.0;

		m.uvs.push_back(Vector2(x * step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step, y * step));

		m.uvs.push_back(Vector2(x * step + step, y * step + step));
		m.uvs.push_back(Vector2(x * step + step, y * step));
		m.uvs.push_back(Vector2(x * step, y * step));
	}

	Texture * texture = Texture::Get("data/textures/flash.tga");

	glColor4f(1, 1, 1, 1.0);

	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); // additive blending

	texture->bind();
	m.render(GL_TRIANGLES);
	texture->unbind();

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

}

void Smoke::update(float elapsed_time)
{
	if (!v_smoke.size())
		return;

	for (int i = 0; i < v_smoke.size(); i++)
	{
		v_smoke[i]->time += elapsed_time;
	}

	Smoke* first = *v_smoke.begin();

	if (first->time >= first->ttl)
	{
		v_smoke.erase(v_smoke.begin());
	}
}