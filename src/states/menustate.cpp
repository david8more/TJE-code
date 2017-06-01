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

Vector2 screen;
float optimer = 0;

void MenuState::init()
{
	// Cargamos texturas de menú
	texture = Texture::Get("data/textures/main.tga");

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	// configuración inicial
	backgroundQuad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	screen = Vector2(game->window_width, game->window_height);
}

void MenuState::onEnter()
{
	cout << "$ Entering menu state" << endl;

	//set the clear color (the background color)
	glClearColor(0.25, 0.25, 0.25, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;

	if (game->bkg_music_playing || !game->music_enabled) return;
	
	SoundManager::getInstance()->playSound("lluvia", true);
	game->bkg_music_playing = true;

}

void MenuState::render() {

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// for screen resizing
	if (screen.x != game->window_width || screen.y != game->window_height)
	{
		backgroundQuad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
		cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
		screen = Vector2(game->window_width, game->window_height);
	}

	cam2D.set();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	// fondo
	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	PG_QUAD.createQuad(inCurrentSelection(0) ? game->window_width*0.225:game->window_width*0.2,
		game->window_height*0.4,
		inCurrentSelection(0) ? 225:180,
		40.f, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(0) ? "data/textures/menu/PG_SELECTION.tga":"data/textures/menu/PG_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	PG_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	HT_QUAD.createQuad(inCurrentSelection(1) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.5,
		inCurrentSelection(1) ? 225 : 180,
		40.f, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(1) ? "data/textures/menu/HT_SELECTION.tga": "data/textures/menu/HT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	HT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	OPT_QUAD.createQuad(inCurrentSelection(2) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.6,
		inCurrentSelection(2) ? 225 : 180,
		40.f, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(2) ? "data/textures/menu/OPT_SELECTION.tga": "data/textures/menu/OPT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	OPT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	E_QUAD.createQuad(inCurrentSelection(3) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.7,
		inCurrentSelection(3) ? 225 : 180,
		40.f, true);
	SELECTION_TEX = Texture::Get(inCurrentSelection(3) ? "data/textures/menu/EX_SELECTION.tga": "data/textures/menu/EX_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	E_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

}

void MenuState::update(double time_elapsed)
{
	Game* game = Game::getInstance();
	
	optimer += time_elapsed;

	if (game->joystick == NULL || optimer < 0.1)
		return;

	JoystickState state = getJoystickState(game->joystick);


	if (state.button[HAT_UP])
	{
		selectionUp();
		optimer = 0;
	}

	else if (state.button[HAT_DOWN])
	{
		selectionDown();
		optimer = 0;
	}

	else if (state.button[A_BUTTON])
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

void MenuState::onLeave(int fut_state) {

	if (fut_state == 3) // selection state
	{
		SoundManager::getInstance()->stopSound("lluvia"); 
		game->bkg_music_playing = false;
	}
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