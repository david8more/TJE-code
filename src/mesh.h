/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	The Mesh contains the info about how to render a mesh and also how to parse it from a file.
*/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <map>
#include "framework.h"
#include "extra/coldet/coldet.h"

class Shader;


class Mesh
{
public:

	typedef struct {
		int num_faces;
		int num_tfaces;
		Vector3 min;
		Vector3 max;
		Vector3 center;
		Vector3 halfsize;
		float radius;
		int vertexMult;
	} sBinHeader;

	sBinHeader header;

	std::vector< Vector3 > vertices; //here we store the vertices
	std::vector< Vector3 > normals;	 //here we store the normals
	std::vector< Vector2 > uvs;	 //here we store the texture coordinates
	std::vector< Vector4 > colors;	 //here we store colors by vertex

	unsigned int vertices_vbo_id;
	unsigned int normals_vbo_id;
	unsigned int uvs_vbo_id;
	unsigned int colors_vbo_id;

	CollisionModel3D* collision_model;

	Mesh();
	Mesh( const Mesh& m );
	~Mesh();

	void clear();
	void render(int primitive);
	void render(int primitive, Shader* sh, unsigned int begin = 0, unsigned int end = 0);

	void uploadToVRAM(); //store the mesh info in the GPU RAM so it renders faster

	void createPlane( float size );
	void createQuad(float center_x, float center_y, float w, float h, bool flip_uvs = false);
	void createBox(float center_x, float center_y, float w, float h, bool flip_uvs = false);

	bool loadASE(const char* filename);

	void createCollisionModel();
	CollisionModel3D* getCollisionModel();
	void setCollisionModel();

	static Mesh* Get(const char* filename);
	static std::map<std::string, Mesh*> s_Meshes;
};

#endif