#include "game.h"
#include "utils.h"
#include "texturemanager.h"
#include "texture.h"

TextureManager* TextureManager::instance = NULL;

TextureManager::TextureManager() {
	instance = this;
}

Texture* TextureManager::getTexture(const char* filename) {
	std::map<std::string, Texture*>::iterator it;

	it = textures.find(filename);

	if (it != textures.end()) return it->second;

	Texture* texture = new Texture();
	if (!texture->load(filename))
	{
		std::cout << "Texture not found: " << filename << std::endl;
		exit(1);
	}

	textures[filename] = texture;

	return texture;
}