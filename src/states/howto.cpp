#include "../includes.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "state.h"
#include "../bass.h"
#include "menustate.h"
#include "playstate.h"
#include "howto.h"
#include <cmath>

#define OBJECTIVE 0
#define KEYBOARD 1
#define XBOX 2

Howto::Howto(StateManager* SManager) : State(SManager) {}
Howto::~Howto() {}

Howto* Howto::getInstance(StateManager* SManager)
{
	static Howto Instance(SManager);
	return &Instance;
}

void Howto::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_DOWN:
	case SDLK_s:
		selectionDown();
		break;
	case SDLK_UP:
	case SDLK_w:
		selectionUp();
		break;
	case SDLK_RETURN:
		selectionChosen();
		break;
	}
}

void Howto::init() {

	texture = Texture::Get("data/textures/blur.tga");

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

	currentSelection = 0;

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

	// menu
	const string submenu_items[] = { "Objetive", "Keyboad", "Xbox Controller", "Back" };

	int p = 0;
	string enable;

	Vector3 c;
	for (int i = 0; i < 4; i++) {
		// highlight current selection
		if (i == currentSelection) c = Vector3(1.f, 1.f, 1.f);
		else c = Vector3(0.25, 0.25, 0.25);
		switch (i)
		{
		case OBJECTIVE:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 2.75);
			break;
		case KEYBOARD:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 2.75);
			break;
		case XBOX:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 2.75);
			break;
		default:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 2.75);
			break;
		}
		p += 35;
	}

	glColor3f(1.f, 1.f, 1.f);
}

void Howto::update(double time_elapsed) {

}
void Howto::selectionUp()
{
	currentSelection--;
	if (currentSelection == -1)
		currentSelection = 3;

	if (!game->effects_enabled)
		return;

	int s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	int s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void Howto::selectionDown()
{
	currentSelection++;
	if (currentSelection == 4)
		currentSelection = 0;

	if (!game->effects_enabled)
		return;

	int s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	int s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void Howto::selectionChosen()
{
	if (game->effects_enabled)
	{
		int s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
		int s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
	}

	if(currentSelection == 3) SManager->changeCurrentState(MenuState::getInstance(SManager));

}

