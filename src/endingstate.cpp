#include "includes.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "endingstate.h"
#include <cmath>

EndingState::EndingState(StateManager* SManager) : State(SManager) {}
EndingState::~EndingState() {}

EndingState* EndingState::getInstance(StateManager* SManager)
{
	static EndingState Instance(SManager);
	return &Instance;
}

void EndingState::onKeyPressed(SDL_KeyboardEvent event)
{
	exit(1);
}

void EndingState::init() {

	texture = new Texture();
	if (!texture->load("data/textures/terrain.tga")){
		cout << "Error: texture has not been loaded" << endl;
		exit(1);
	}

	game = Game::getInstance();

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
}

void EndingState::onEnter()
{
	cout << "$ Entering 'how to play' state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void EndingState::onLeave(int fut_state)
{

}

void EndingState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

void EndingState::update(double time_elapsed) {

}
