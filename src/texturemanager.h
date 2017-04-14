#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include "texture.h"
#include <map>

class TextureManager {

public:
	// SINGLETON
	static TextureManager* instance;

	static TextureManager* getInstance() {
		if (instance == NULL) instance = new TextureManager();
		return instance;
	}

	Texture* getTexture(const char* filename);

private:
	TextureManager();
	std::map<std::string, Texture*> textures;
};

#endif // !TEXTUREMANAGER_H
