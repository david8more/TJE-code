#ifndef MESHMANAGER_H
#define MESHMANAGER_H
#include "mesh.h"
#include <map>

class MeshManager {

public:
	// SINGLETON
	static MeshManager* instance;

	static MeshManager* getInstance() {
		if (instance == NULL) instance = new MeshManager();
		return instance;
	}

	Mesh* getMesh(const char* filename, bool createBox);

private:
	MeshManager();
	std::map<std::string, Mesh*> meshes;
};

#endif // !MESHMANAGER_H
