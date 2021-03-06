#include "mesh.h"
#include <cassert>
#include "includes.h"
#include "utils.h"
#include "shader.h"
#include "extra/textparser.h"
#include <iostream>
#include <vector>

#define DEBUG 0

std::map<std::string, Mesh*> Mesh::s_Meshes;

Mesh::Mesh()
{
	vertices_vbo_id = 0;
	normals_vbo_id = 0;
	uvs_vbo_id = 0;
	colors_vbo_id = 0;
	collision_model = NULL;
}

Mesh::Mesh( const Mesh& m )
{
	vertices = m.vertices;
	normals = m.normals;
	uvs = m.uvs;
	colors = m.colors;
	collision_model = NULL;
}

Mesh::~Mesh()
{
	if(vertices_vbo_id) glDeleteBuffersARB(1, &vertices_vbo_id);
	if(normals_vbo_id) glDeleteBuffersARB(1, &normals_vbo_id);
	if(uvs_vbo_id) glDeleteBuffersARB(1, &uvs_vbo_id);
	if(colors_vbo_id) glDeleteBuffersARB(1, &colors_vbo_id);
}

void Mesh::clear()
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();
}


void Mesh::render(int primitive)
{
	assert(vertices.size() && "No vertices in this mesh");

    glEnableClientState(GL_VERTEX_ARRAY);

	if(vertices_vbo_id)
	{
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, vertices_vbo_id );
		glVertexPointer(3, GL_FLOAT, 0, NULL );
	}
	else
		glVertexPointer(3, GL_FLOAT, 0, &vertices[0] );

	if (normals.size())
	{
		glEnableClientState(GL_NORMAL_ARRAY);
		if(normals_vbo_id)
		{
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, normals_vbo_id );
			glNormalPointer(GL_FLOAT, 0, NULL );
		}
		else
			glNormalPointer(GL_FLOAT, 0, &normals[0] );
	}

	if (uvs.size())
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(uvs_vbo_id)
		{
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, uvs_vbo_id );
			glTexCoordPointer(2, GL_FLOAT, 0, NULL );
		}
		else
			glTexCoordPointer(2, GL_FLOAT, 0, &uvs[0] );
	}

	if (colors.size())
	{
		glEnableClientState(GL_COLOR_ARRAY);
		if(colors_vbo_id)
		{
			glBindBufferARB( GL_ARRAY_BUFFER_ARB, colors_vbo_id );
			glColorPointer(4, GL_FLOAT, 0, NULL );
		}
		else
			glColorPointer(4, GL_FLOAT, 0, &colors[0] );
	}
    
	glDrawArrays(primitive, 0, (GLsizei)vertices.size() );
	glDisableClientState(GL_VERTEX_ARRAY);

	if (normals.size())
		glDisableClientState(GL_NORMAL_ARRAY);
	if (uvs.size())
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	if (colors.size())
		glDisableClientState(GL_COLOR_ARRAY);
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
}

void Mesh::render( int primitive, Shader* sh,  unsigned int begin, unsigned int end)
{
	if(!sh || !sh->compiled)
        return render(primitive);
    
	assert(vertices.size() && "No vertices in this mesh");

    int vertex_location = sh->getAttribLocation("a_vertex");
	assert(vertex_location != -1 && "No a_vertex found in shader");

	if(vertex_location == -1)
		return;

    glEnableVertexAttribArray(vertex_location);
	if(vertices_vbo_id)
	{
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, vertices_vbo_id );
		glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	}
	else
		glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, &vertices[0] );

	int normal_location = -1;
    if (normals.size())
    {
        normal_location = sh->getAttribLocation("a_normal");
        if(normal_location != -1)
        {
            glEnableVertexAttribArray(normal_location);
			if(normals_vbo_id)
			{
				glBindBufferARB( GL_ARRAY_BUFFER_ARB, normals_vbo_id );
	            glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, &normals[0] );
        }
    }
    
	int uv_location = -1;
	if (uvs.size())
	{
        uv_location = sh->getAttribLocation("a_uv");
        if(uv_location != -1)
        {
            glEnableVertexAttribArray(uv_location);
			if(uvs_vbo_id)
			{
				glBindBufferARB( GL_ARRAY_BUFFER_ARB, uvs_vbo_id );
	            glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(uv_location, 2, GL_FLOAT, GL_FALSE, 0, &uvs[0] );
        }
    }
    
	int color_location = -1;
	if (colors.size())
	{
        color_location = sh->getAttribLocation("a_color");
        if(color_location != -1)
        {
            glEnableVertexAttribArray(color_location);
			if(colors_vbo_id)
			{
				glBindBufferARB( GL_ARRAY_BUFFER_ARB, colors_vbo_id );
	            glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, 0, NULL );
			}
			else
	            glVertexAttribPointer(color_location, 4, GL_FLOAT, GL_FALSE, 0, &colors[0] );
        }
    }
    
	if(!begin && !end)
		glDrawArrays(primitive, 0, (GLsizei)vertices.size() );
	else 
		glDrawArrays(primitive, (GLsizei)begin, (GLsizei)end);

	glDisableVertexAttribArray( vertex_location );
	if(normal_location != -1) glDisableVertexAttribArray( normal_location );
	if(uv_location != -1) glDisableVertexAttribArray( uv_location );
	if(color_location != -1) glDisableVertexAttribArray( color_location );
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, 0 );
}

void Mesh::uploadToVRAM()
{
	if (glGenBuffersARB == 0)
	{
		std::cout << "Error: your graphics cards dont support VBOs. Sorry." << std::endl;
		exit(0);
	}

	//delete old
	if(vertices_vbo_id) glDeleteBuffersARB(1, &vertices_vbo_id);
	if(normals_vbo_id) glDeleteBuffersARB(1, &normals_vbo_id);
	if(uvs_vbo_id) glDeleteBuffersARB(1, &uvs_vbo_id);
	if(colors_vbo_id) glDeleteBuffersARB(1, &colors_vbo_id);

	glGenBuffersARB( 1, &vertices_vbo_id ); //generate one handler (id)
	glBindBufferARB( GL_ARRAY_BUFFER_ARB, vertices_vbo_id ); //bind the handler
	glBufferDataARB( GL_ARRAY_BUFFER_ARB, vertices.size() * 3 * sizeof(float), &vertices[0], GL_STATIC_DRAW_ARB ); //upload data

	if(normals.size())
	{
		glGenBuffersARB( 1, &normals_vbo_id); //generate one handler (id)
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, normals_vbo_id ); //bind the handler
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, normals.size() * 3 * sizeof(float), &normals[0], GL_STATIC_DRAW_ARB ); //upload data
	}

	if(uvs.size())
	{
		glGenBuffersARB( 1, &uvs_vbo_id); //generate one handler (id)
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, uvs_vbo_id ); //bind the handler
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, uvs.size() * 2 * sizeof(float), &uvs[0], GL_STATIC_DRAW_ARB ); //upload data
	}

	if(colors.size())
	{
		glGenBuffersARB( 1, &colors_vbo_id); //generate one handler (id)
		glBindBufferARB( GL_ARRAY_BUFFER_ARB, colors_vbo_id ); //bind the handler
		glBufferDataARB( GL_ARRAY_BUFFER_ARB, colors.size() * 4 * sizeof(float), &colors[0], GL_STATIC_DRAW_ARB ); //upload data
	}

}

void Mesh::createQuad(float center_x, float center_y, float w, float h, bool flip_uvs )
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back( Vector3( center_x + w*0.5f, center_y + h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x + w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y + h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x - w*0.5f, center_y - h*0.5f, 0.0f ) );
	vertices.push_back( Vector3( center_x + w*0.5f, center_y + h*0.5f, 0.0f ) );

	//texture coordinates
	uvs.push_back( Vector2(1.0f,flip_uvs ? 0.0f : 1.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(1.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 0.0f : 1.0f) );
	uvs.push_back( Vector2(0.0f,flip_uvs ? 1.0f : 0.0f) );
	uvs.push_back( Vector2(1.0f,flip_uvs ? 0.0f : 1.0f) );

	//all of them have the same normal
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
	normals.push_back( Vector3(0.0f,0.0f,1.0f) );
}

void Mesh::createBox(float center_x, float center_y, float w, float h, bool flip_uvs)
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back(Vector3(center_x + w*0.5f, center_y - h*0.5f, 0.0f));
	vertices.push_back(Vector3(center_x - w*0.5f, center_y - h*0.5f, 0.0f));

	vertices.push_back(Vector3(center_x - w*0.5f, center_y - h*0.5f, 0.0f));
	vertices.push_back(Vector3(center_x - w*0.5f, center_y + h*0.5f, 0.0f));

	vertices.push_back(Vector3(center_x - w*0.5f, center_y + h*0.5f, 0.0f));
	vertices.push_back(Vector3(center_x + w*0.5f, center_y + h*0.5f, 0.0f));

	vertices.push_back(Vector3(center_x + w*0.5f, center_y + h*0.5f, 0.0f));
	vertices.push_back(Vector3(center_x + w*0.5f, center_y - h*0.5f, 0.0f));

	//texture coordinates
	uvs.push_back(Vector2(1.0f, flip_uvs ? 0.0f : 1.0f));
	uvs.push_back(Vector2(0.0f, flip_uvs ? 1.0f : 0.0f));
	uvs.push_back(Vector2(1.0f, flip_uvs ? 1.0f : 0.0f));
	uvs.push_back(Vector2(0.0f, flip_uvs ? 0.0f : 1.0f));
	//uvs.push_back(Vector2(0.0f, flip_uvs ? 1.0f : 0.0f));
	//uvs.push_back(Vector2(1.0f, flip_uvs ? 0.0f : 1.0f));

	//all of them have the same normal
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	//normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	//normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
}


void Mesh::createPlane(float size)
{
	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	//create six vertices (3 for upperleft triangle and 3 for lowerright)

	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(size,0,-size) );
	vertices.push_back( Vector3(-size,0,-size) );
	vertices.push_back( Vector3(-size,0,size) );
	vertices.push_back( Vector3(size,0,size) );
	vertices.push_back( Vector3(-size,0,-size) );

	//all of them have the same normal
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );
	normals.push_back( Vector3(0,1,0) );

	//texture coordinates
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(1,0) );
	uvs.push_back( Vector2(0,0) );
	uvs.push_back( Vector2(0,1) );
	uvs.push_back( Vector2(1,1) );
	uvs.push_back( Vector2(0,0) );
}


bool Mesh::loadASE(const char* name) {

	vertices.clear();
	normals.clear();
	uvs.clear();
	colors.clear();

	long time = getTime();

	std::string filename(name);

	std::string filename_bin = "data/binaries/";
	filename_bin += filename;
	filename_bin += ".bin";

	FILE* file = fopen(filename_bin.c_str(), "rb");

	if (file != NULL && 1) {
		fread(&header, sizeof(sBinHeader), 1, file);
		
		vertices.resize(header.num_faces * 3);
		uvs.resize(header.num_tfaces * 3);
		normals.resize(header.num_tfaces * 3);
		
		fread(&vertices[0], sizeof(Vector3), header.num_faces * 3, file);
		fread(&normals[0], sizeof(Vector3), header.num_tfaces * 3, file);
		fread(&uvs[0], sizeof(Vector2), header.num_tfaces * 3, file);
		fclose(file);

		uploadToVRAM();

		long time0 = getTime();
		if (DEBUG) std::cout << "Parsing time " << (time0 - time) * 0.001 << std::endl;

		return true;
	}

	TextParser t;
	std::string filename_ase = "data/ase/";
	filename_ase += filename;

	if (!t.create(filename_ase.c_str())) {
		std::cout << "File not found " << filename << std::endl;
		return false;
	}

	std::vector< Vector3 > unique_vertices;

	t.seek("*MESH_NUMVERTEX");
	int num_vertex = t.getint();
	if (DEBUG) std::cout << "#Mesh vertex: " << num_vertex << std::endl;

	t.seek("*MESH_NUMFACES");
	int num_faces = t.getint();
	if (DEBUG) std::cout << "#Mesh faces: " << num_faces << std::endl;

	Vector3 max_v (-1000000, -10000000, -10000000);
	Vector3 min_v (1000000, 10000000, 10000000);

	unique_vertices.resize(num_vertex);
	
	for (int i = 0; i < num_vertex; i++) {
		t.seek("*MESH_VERTEX");
		t.getint();

		float x = t.getfloat();
		float y = -t.getfloat();
		float z = t.getfloat();

		Vector3 v(x, z, y);

		if (v.x < min_v.x)
			min_v.x = v.x;
		if (v.y < min_v.y)
			min_v.y = v.y;
		if (v.z < min_v.z)
			min_v.z = v.z;

		if (v.x > max_v.x)
			max_v.x = v.x;
		if (v.y > max_v.y)
			max_v.y = v.y;
		if (v.z > max_v.z)
			max_v.z = v.z;

		unique_vertices[i] = v;
	}

	//int current_multitexture = 0;

	for (int i = 0; i < num_faces; ++i) {
		t.seek("*MESH_FACE");
		t.getword();
		t.getword();
		int A = t.getint();
		t.getword();
		int B = t.getint();
		t.getword();
		int C = t.getint();

		vertices.push_back(unique_vertices[A]);
		vertices.push_back(unique_vertices[B]);
		vertices.push_back(unique_vertices[C]);

		/*t.seek("*MESH_MTLID");
		if (t.getint())
		{
			current_multitexture = t.getint();
		}*/
	}

	//header.vertexMult = current_multitexture;


	// --------------------------------------------------------------

	t.seek("*MESH_NUMTVERTEX");
	int num_tvertex = t.getint();
	if (DEBUG) std::cout << "#Mesh t-vertex: " << num_tvertex << std::endl;

	std::vector< Vector2 > unique_tvertices;
	unique_tvertices.resize(num_tvertex);

	// coordenadas vertices de textura
	for (int i = 0; i < num_tvertex; i++) {
		t.seek("*MESH_TVERT");
		t.getint();

		float cu = t.getfloat();
		float cv = t.getfloat();

		Vector2 v(cu, cv);
		unique_tvertices[i] = v;
	}

	t.seek("*MESH_NUMTVFACES");
	int num_tfaces = t.getint();
	if (DEBUG) std::cout << "#Mesh t-faces: " << num_tfaces << std::endl;

	for (int i = 0; i < num_tfaces; ++i) {
		t.seek("*MESH_TFACE");
		t.getint();
		int A = t.getint();
		int B = t.getint();
		int C = t.getint();

		uvs.push_back(unique_tvertices[A]);
		uvs.push_back(unique_tvertices[B]);
		uvs.push_back(unique_tvertices[C]);

	}

	normals.resize(num_tfaces * 3);

	for (int i = 0; i < num_tfaces*3; i++) {
		t.seek("*MESH_VERTEXNORMAL");
		t.getint();
		float A = t.getfloat();
		float B = -t.getfloat();
		float C = t.getfloat();

		Vector3 N1(A, C, B);

		normals[i] = N1;
	}

	long time2 = getTime();
	if(DEBUG) std::cout << "Parsing time " << (time2 - time) * 0.001 << std::endl;

	header.num_faces = num_faces;
	header.num_tfaces = num_tfaces;

	header.min = min_v;
	header.max = max_v;

	header.center = ((header.max + header.min) * 0.5);
	header.halfsize = (header.max - header.center);
	header.radius = header.halfsize.length();

	FILE* wfile = fopen(filename_bin.c_str(), "wb");

	fwrite(&header, sizeof(header), 1, wfile);
	fwrite(&vertices[0], sizeof(Vector3), vertices.size(), wfile);
	fwrite(&normals[0], sizeof(Vector3), normals.size(), wfile);
	fwrite(&uvs[0], sizeof(Vector2), uvs.size(), wfile);

	fclose(wfile);
	uploadToVRAM();

	return true;
}

Mesh* Mesh::Get(const char* filename)
{
	std::string name = std::string(filename);
	std::map<std::string, Mesh*>::iterator it = s_Meshes.find(name);
	if (it != s_Meshes.end())
		return it->second;

	Mesh* m = new Mesh();
	if (!m->loadASE(filename))
		return NULL;
	s_Meshes[name] = m;
	return m;
}

// colisiones (coldet)
void Mesh::createCollisionModel() {

	//para crear una instancia usamos la funci�n newCollisionModel3D ya que coldet no permite hacer un new directamente CollisionModel3D*
	this->collision_model = newCollisionModel3D();

	//esto acelera el proceso si sabemos cuantos triangulos hay porque evita tener que ir reallocando los datos
	// num triangulos = num vertices / 3 (por las caras!!!)
	this->collision_model->setTriangleNumber(vertices.size() / 3);

	//agregamos uno a uno todos los triangulos de la mesh pasandole las coordenadas de los tres vertices que forman cada triangulo
	for (int i = 0; i < vertices.size(); i += 3) {
		this->collision_model->addTriangle(vertices[i].x, vertices[i].y, vertices[i].z,
			vertices[i+1].x, vertices[i+1].y, vertices[i+1].z,
			vertices[i+2].x, vertices[i+2].y, vertices[i+2].z);
	}

	//una vez tiene todos los triangulos llamamos a finalize para que cree el arbol interno optimizado
	this->collision_model->finalize();
	
}

CollisionModel3D * Mesh::getCollisionModel()
{
	if (this->collision_model != NULL)
		return collision_model;

	std::cout << " Creando collision model" << std::endl;
	createCollisionModel();
	return collision_model; 
}

void Mesh::setCollisionModel() {
	if (this->collision_model == NULL)
		createCollisionModel();
}

