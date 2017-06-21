#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../bass.h"
#include "../soundmanager.h"
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

float frame_ttl = 0.125;

void LoadingState::onEnter()
{
	//set the clear color (the background color)
	glClearColor(1.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	game = Game::getInstance();
	backgroundQuad.createQuad(game->window_width*0.5, game->window_height*0.5, game->window_width, game->window_height, true);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);

	game->joystick = openJoystick(0);

	current_ttl = frame_ttl;
	iterator = 0;

	std::stringstream ss;
	for (int i = 1; i < 571; i++)
	{
		ss << "data/textures/cinematic/";
		ss << "scene (" << i << ").tga";
		slider.push_back(ss.str());
		ss.str("");
	}
}

void LoadingState::onLeave(int fut_state)
{

}

void LoadingState::render()
{
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();

	texture = Texture::Get(slider[iterator].c_str());

	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	glColor4f(1.f, 1.f, 1.f, 1.f);
	std::string text = "[Press any key to skip intro]";
	drawText(game->window_width*0.5 - game->window_height*text.size()*0.008, game->window_height*0.75, text, Vector3(1.f, 1.f, 1.f), game->window_height*0.003);
}

void LoadingState::update(double time_elapsed)
{
	Game* game = Game::getInstance();

	if (game->joystick != NULL)
	{
		JoystickState state = getJoystickState(game->joystick);

		for (int i = 0; i < 14; i++)
		{
			if (state.button[i])
			{
				Load();
				return;
			}

		}
	}

	current_ttl -= time_elapsed;

	if (current_ttl < 0)
	{
		iterator++;
		current_ttl = frame_ttl;
		if (iterator == slider.size())
			Load();
	}
}

void LoadingState::onKeyPressed(SDL_KeyboardEvent event)
{
	if (event.keysym.sym == SDLK_ESCAPE)
		return;

	SoundManager::getInstance()->stopSound("cinematic");
	Load();
}

void LoadingState::Load()
{
	MenuState::getInstance(this->SManager)->init();
	OptionsState::getInstance(this->SManager)->init();
	Howto::getInstance(this->SManager)->init();
	SelectionState::getInstance(this->SManager)->init();
	PlayState::getInstance(this->SManager)->init();

	SManager->changeCurrentState(MenuState::getInstance(SManager));
}
