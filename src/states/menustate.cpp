#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../includes.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../bass.h"
#include "optionsstate.h"
#include "menustate.h"
#include "playstate.h"
#include "selectionstate.h"
#include "howto.h"
#include <cmath>
#include <ctime>
#include "../soundmanager.h"

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

float optimer = 0;

void MenuState::init()
{
	// Cargamos texturas de men�
	texture = Texture::Get("data/textures/main.tga");
	// Cogemos la instancia de game para no hacerlo en cada m�todo
	game = Game::getInstance();
}

void MenuState::onEnter()
{
	cout << "$ Entering menu state" << endl;

	//set the clear color (the background color)
	glClearColor(0.25, 0.25, 0.25, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;
	// siempre q vayamos al men� que se reinicie
	game->end = true;

	if (!game->bkg_music_playing && game->music_enabled)
	{
		SoundManager::getInstance()->playSound("music", true);
		game->bkg_music_playing = true;
	}
}

void MenuState::render()
{
	int w = game->window_width;
	int h = game->window_height;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	backgroundQuad.createQuad(w * 0.5, h * 0.5, w, h, true);
	cam2D.setOrthographic(0.0, w, h, 0.0, -1.0, 1.0);
	cam2D.set();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	// fondo
	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	PG_QUAD.createQuad(inCurrentSelection(0) ? w*0.2 + sin(game->time) * 10 : w*0.2,
		h*0.4,
		inCurrentSelection(0) ? 245:200,
		75, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(0) ? "data/textures/menu/PG_SELECTION.tga":"data/textures/menu/PG_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	PG_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	HT_QUAD.createQuad(inCurrentSelection(1) ? w*0.2 + sin(game->time) * 10 : w*0.2,
		h*0.5,
		inCurrentSelection(1) ? 245 : 200,
		75, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(1) ? "data/textures/menu/HT_SELECTION.tga": "data/textures/menu/HT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	HT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	OPT_QUAD.createQuad(inCurrentSelection(2) ? w*0.2 + sin(game->time) * 10 : w*0.2,
		h*0.6,
		inCurrentSelection(2) ? 245 : 200,
		75, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(2) ? "data/textures/menu/OPT_SELECTION.tga": "data/textures/menu/OPT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	OPT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	E_QUAD.createQuad(inCurrentSelection(3) ? w*0.2 + sin(game->time) * 10 : w*0.2,
		h*0.7,
		inCurrentSelection(3) ? 245 : 200,
		75, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(3) ? "data/textures/menu/EX_SELECTION.tga": "data/textures/menu/EX_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	E_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	glDisable(GL_BLEND);
}

void MenuState::update(double time_elapsed)
{
	Game* game = Game::getInstance();
	
	optimer += time_elapsed;

	if (game->joystick == NULL)
		return;

	JoystickState state = getJoystickState(game->joystick);

	if (state.axis[LEFT_ANALOG_Y] > 0.2 && optimer > 0.2)
	{
		selectionDown();
		optimer = 0;
	}

	if (state.axis[LEFT_ANALOG_Y] < -0.2 && optimer > 0.2)
	{
		selectionUp();
		optimer = 0;
	}

	if (state.button[A_BUTTON] && optimer > 0.2)
	{
		selectionChosen();
		optimer = 0;
	}

}

bool MenuState::inCurrentSelection(int selection) {
	if (currentSelection == selection) return true;
	else return false;
}

void MenuState::onKeyPressed(SDL_KeyboardEvent event)
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

void MenuState::onLeave(int fut_state)
{

}

void MenuState::selectionUp()
{
	currentSelection--;
	if (currentSelection == -1)
		currentSelection = 3;

	if (game->effects_enabled)
		SoundManager::getInstance()->playSound("move_menu", false);
}

void MenuState::selectionDown()
{
	currentSelection++;
	if (currentSelection == 4)
		currentSelection = 0;

	if (game->effects_enabled)
		SoundManager::getInstance()->playSound("move_menu", false);
}

void MenuState::selectionChosen()
{
	if (game->effects_enabled)
	{
		SoundManager::getInstance()->playSound("move_menu", false);
	}

	switch (currentSelection)
	{
	case PLAYSTATE:
		SManager->changeCurrentState(SelectionState::getInstance(SManager));
		break;
	case HOWTOPLAY:
		SManager->changeCurrentState(Howto::getInstance(SManager));
		break;
	case OPTIONS:
		SManager->changeCurrentState(OptionsState::getInstance(SManager));
		break;
	default:
		exit(1);
		break;
	}
}