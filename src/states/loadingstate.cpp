#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../bass.h"
#include "menustate.h"
#include "playstate.h"
#include "optionsstate.h"
#include "selectionstate.h"
#include "loadingstate.h"
#include "endingstate.h"
#include "howto.h"
#include <cmath>
#include <ctime>

LoadingState::LoadingState(StateManager* SManager) : State(SManager) {}
LoadingState::~LoadingState() {}

void LoadingState::onEnter()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	texture = Texture::Get("data/textures/blur.tga");
	
	game = Game::getInstance();
	backgroundQuad.createQuad(game->window_width*0.5, game->window_height*0.5, game->window_width, game->window_height, true);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
}

void LoadingState::onLeave(int fut_state)
{

}

void LoadingState::render()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();

	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	glColor4f(1.f, 1.f, 1.f, 1.f);
	std::string text = "[Press any key to continue]";
	drawText(game->window_width*0.5 - game->window_height*text.size()*0.008, game->window_height*0.75, text, Vector3(1.f, 1.f, 1.f), game->window_height*0.003);
}

void LoadingState::onKeyPressed(SDL_KeyboardEvent event)
{
	if (event.keysym.sym == SDLK_ESCAPE) return;

	MenuState::getInstance(this->SManager)->init();
	OptionsState::getInstance(this->SManager)->init();
	Howto::getInstance(this->SManager)->init();
	EndingState::getInstance(this->SManager)->init();
	SelectionState::getInstance(this->SManager)->init();
	PlayState::getInstance(this->SManager)->init();

	SManager->changeCurrentState(MenuState::getInstance(SManager));
}
