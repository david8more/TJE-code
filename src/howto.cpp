#include "includes.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "texturemanager.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "menustate.h"
#include "playstate.h"
#include "howto.h"
#include <cmath>

Howto::Howto(StateManager* SManager) : State(SManager) {}
Howto::~Howto() {}

Howto* Howto::getInstance(StateManager* SManager)
{
	static Howto Instance(SManager);
	return &Instance;
}

void Howto::onKeyPressed(SDL_KeyboardEvent event)
{
	SManager->changeCurrentState(MenuState::getInstance(SManager));

	switch (event.keysym.sym)
	{

	}
}

void Howto::init() {

	texture = TextureManager::getInstance()->getTexture("data/textures/terrain.tga");

	game = Game::getInstance();

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
}

void Howto::onEnter()
{
	cout << "$ Entering 'how to play' state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void Howto::onLeave(int fut_state)
{

}

void Howto::render() {

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	glEnable(GL_BLEND);
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();
	glDisable(GL_BLEND);
}

void Howto::update(double time_elapsed) {

}
