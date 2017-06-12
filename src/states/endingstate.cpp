#include "../includes.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "state.h"
#include "../bass.h"
#include "endingstate.h"
#include "menustate.h"
#include "../world.h"
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
	SManager->changeCurrentState(MenuState::getInstance(SManager));
}

void EndingState::init()
{
	Game * game = Game::instance;

	int w = game->window_width;
	int h = game->window_height;

	texture = Texture::Get("data/textures/blur.tga");

	cam2D.setOrthographic(0.0, w, h, 0.0, -1.0, 1.0);
	quad.createQuad(w * 0.5, h * 0.5, w, h, true);
}

void EndingState::onEnter()
{
	cout << "$ Entering 'ending' state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.f, 1.f, 1.f);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void EndingState::onLeave(int fut_state)
{
	
}

void EndingState::render()
{
	Game * game = Game::instance;
	int score = game->score;

	int w = game->window_width;
	int h = game->window_height;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	glEnable(GL_BLEND);
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();
	glDisable(GL_BLEND);

	glColor3f(1.f, 1.f, 1.f);

	std::stringstream ss;

	ss << "SCORE: " << game->score;

	drawText(w * 0.5, h * 0.5, ss.str(), Vector3(1.0, 1.0, 1.0), 5.0);

	ss.str("");
	int diff = game->difficulty;
	std::string rank;

	switch (diff)
	{
	case D_BABY:

		if (score > 750)
			rank =  "YOU ARE NOW FIRST LIEUTENANT";
		if (score > 1000)
			rank = "YOU ARE NOW CAPTAIN";
		if (score > 1250)
			rank = "YOU ARE NOW MAJOR";
		if(score > 1500)
			rank = "YOU ARE NOW COLONEL";

		drawText(w * 0.5, h * 0.7, rank, Vector3(1.0, 1.0, 1.0), 5.0);
		break;
	case D_SKILLED:

		if (score > 1625)
			rank = "YOU ARE NOW FIRST LIEUTENANT";
		if (score > 2150)
			rank = "YOU ARE NOW CAPTAIN";
		if (score > 2750)
			rank = "YOU ARE NOW MAJOR";
		if (score > 3250)
			rank = "YOU ARE NOW COLONEL";

		drawText(w * 0.5, h * 0.7, rank, Vector3(1.0, 1.0, 1.0), 5.0);
		break;
	case D_INSANE:

		if (score > 2600)
			rank = "YOU ARE NOW FIRST LIEUTENANT";
		if (score > 3500)
			rank = "YOU ARE NOW CAPTAIN";
		if (score > 4350)
			rank = "YOU ARE NOW MAJOR";
		if (score > 5250)
			rank = "YOU ARE NOW COLONEL";

		drawText(w * 0.5, h * 0.7, rank, Vector3(1.0, 1.0, 1.0), 5.0);
		break;
	default:
		break;
	}


}

void EndingState::update(double time_elapsed) {

}
