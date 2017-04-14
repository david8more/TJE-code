#include "game.h"
#include "utils.h"
#include "meshmanager.h"
#include "mesh.h"

MeshManager* MeshManager::instance = NULL;

MeshManager::MeshManager()
{
	instance = this;
}

Mesh* MeshManager::getMesh(const char* filename, bool createBox)
{
	std::map<std::string, Mesh*>::iterator it;

	it = meshes.find(filename);

	if (it != meshes.end()) return it->second;

	Mesh* mesh = new Mesh();
	if (!mesh->loadASE(filename, createBox))
	{
		std::cout << "ASE not found: " << filename << std::endl;
		exit(1);
	}

	meshes[filename] = mesh;

	return mesh;
}