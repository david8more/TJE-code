#include "explosion.h"
#include "mesh.h"
#include "entity.h"
#include "texture.h"
#include "framework.h"

std::vector<Explosion*> Explosion::v_explosions;

Explosion::Explosion(Vector3 collisionPoint)
{
	collision = collisionPoint;
    time = 0.0;
    lifetime = 3.5;
}

Explosion::~Explosion() {

}

void Explosion::createExplosion(Vector3 atCollision)
{
    Explosion * newExplosion = new Explosion(atCollision);
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

        float w = 10.0, h = 10.0;
        Vector3 ep = current->collision;
		Vector3 up = camera->up;
		Vector3 side = (camera->center - camera->eye).cross(camera->up);

        up.normalize();
        side.normalize();

		Vector3 Hm = up*h*0.5;
		Vector3 Wm = side*w*0.5;
        					
        m.vertices.push_back( ep+(Hm + Wm));
        m.vertices.push_back( ep+(-Hm + Wm));
        m.vertices.push_back( ep+(-Hm - Wm));

        m.vertices.push_back( ep+(Hm-Wm));
        m.vertices.push_back( ep+(Hm + Wm));
        m.vertices.push_back( ep+(-Hm - Wm));

        float f = current->time / current->lifetime;
        unsigned frame = f * 25.0;

        float xt = frame % 5, yt = frame / 5;
        float step = 0.2f;

        m.uvs.push_back(Vector2(xt*step, yt*step + step));
        m.uvs.push_back(Vector2(xt*step + step, yt*step + step));
        m.uvs.push_back(Vector2(xt*step, yt*step));
        m.uvs.push_back(Vector2(xt*step + step, yt*step +step));
        m.uvs.push_back(Vector2(xt*step + step, yt*step));
        m.uvs.push_back(Vector2(xt*step, yt*step));
    }

	Texture * texture = Texture::Get("data/textures/explosion.tga");

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

    if(first->time >= first->lifetime)
	{
		v_explosions.erase(v_explosions.begin());
    }
}