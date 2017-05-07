#include "../includes.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../bass.h"
#include "../entity.h"
#include "state.h"
#include "menustate.h"
#include "playstate.h"
#include "howto.h"
#include <cmath>

#define OBJECTIVE 0
#define KEYBOARD 1
#define XBOX 2
#define BACK 3

Camera cam;
Mesh quead;
float httimer = 0;

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

	cam.setOrthographic(0.0, 600, 600, 0.0, -1.0, 1.0);
	
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
	
	std::string current_howto = "";

	cam2D.set();

	switch (currentSelection) {
	case OBJECTIVE:
		current_howto = "data/textures/howto-obj1.tga";
		break;
	case KEYBOARD:
		current_howto = "data/textures/howto-keyboard.tga";
		break;
	case XBOX:
		current_howto = "data/textures/howto-xb.tga";
		break;
	default:
		current_howto = "data/textures/blur.tga";
		break;
	}

	texture = Texture::Get(current_howto.c_str());
	
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();
		

	// menu
	const string submenu_items[] = { "Objetive", "Keyboard", "Xbox Controller", "Back" };

	int p = 0;

	glColor4f(1.0, 1.0, 1.0, 1.0);

	Vector3 c;
	for (int i = 0; i < 4; i++) {
		// highlight current selection
		if (i == currentSelection) c = Vector3(1.f, 1.f, 1.f);
		else c = Vector3(0.25, 0.25, 0.25);
		switch (i)
		{
		case OBJECTIVE:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 2.75);
			break;
		case KEYBOARD:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 2.75);
			break;
		case XBOX:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 2.75);
			break;
		default:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 2.75);
			break;
		}
		p += 35;
	}

	glColor4f(1.0, 1.0, 1.0, 0.0);
}

void Howto::update(double time_elapsed)
{
	Game* game = Game::getInstance();

	if (game->joystick == NULL)
		return;

	JoystickState state = getJoystickState(game->joystick);

	httimer += time_elapsed;

	if (state.button[HAT_UP] && httimer > 0.1)
	{
		selectionUp();
		httimer = 0;
	}

	if (state.button[HAT_DOWN] && httimer > 0.1)
	{
		selectionDown();
		httimer = 0;
	}

	if (state.button[A_BUTTON] && httimer > 0.2)
	{
		httimer = 0;
		selectionChosen();
	}

	if (state.button[B_BUTTON] && httimer > 0.2)
	{
		httimer = 0;
		SManager->changeCurrentState(MenuState::getInstance(SManager));
	}
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

	if(currentSelection == BACK)
		SManager->changeCurrentState(MenuState::getInstance(SManager));

}

