#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "texturemanager.h"
#include "shader.h"
#include "optionsstate.h"
#include "bass.h"
#include "menustate.h"
#include "playstate.h"
#include "preplay.h"
#include "howto.h"
#include <cmath>
#include <ctime>

float x = 200.f; float y = 200.f;

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

void MenuState::init() {

	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

	// Cargamos texturas de menú
	texture = TextureManager::getInstance()->getTexture("data/textures/mainv2.tga");
	smokeTexture = TextureManager::getInstance()->getTexture("data/textures/smoke_alpha.tga");

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	// configuración inicial
	backgroundQuad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
}

void MenuState::onEnter()
{
	cout << "$ Entering menu state" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;

	if (game->bkg_music_playing != true && game->music_enabled) {
		b_sample = BASS_SampleLoad(false, "data/sounds/lluvia.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
		b_channel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
		BASS_ChannelPlay(b_channel, false); // play it
		game->bkg_music_playing = true;
	}
}

void MenuState::render() {

	if (rand() % 3) x+=0.25;
	else x-=0.25;

	if (rand() % 2) y+=0.25;
	else y-=0.25;

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	glColor4f(1.f, 1.f, 1.f, 0.f);
	
	// fondo
	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	// smoke
	smoke_quad.createQuad(x, y, game->window_width, game->window_height, true);
	smoke_quad2.createQuad(x + 100.f, y + 150.f, game->window_width, game->window_height, true);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	smokeTexture->bind();
	smoke_quad.render(GL_TRIANGLES);
	smoke_quad2.render(GL_TRIANGLES);
	smokeTexture->unbind();
	glDisable(GL_BLEND);

	PG_QUAD.createQuad(inCurrentSelection(0) ? game->window_width*0.225:game->window_width*0.2,
		game->window_height*0.4,
		inCurrentSelection(0) ? game->window_width*0.25: game->window_width*0.2,
		game->window_height*0.075, true);
	SELECTION_TEX = TextureManager::getInstance()->getTexture(inCurrentSelection(0) ? "data/textures/menu/PG_SELECTION.tga":"data/textures/menu/PG_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	PG_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	HT_QUAD.createQuad(inCurrentSelection(1) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.5,
		inCurrentSelection(1) ? game->window_width*0.25 : game->window_width*0.2,
		game->window_height*0.075, true);	SELECTION_TEX = TextureManager::getInstance()->getTexture(inCurrentSelection(1) ? "data/textures/menu/HT_SELECTION.tga": "data/textures/menu/HT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	HT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	OPT_QUAD.createQuad(inCurrentSelection(2) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.6,
		inCurrentSelection(2) ? game->window_width*0.25 : game->window_width*0.2,
		game->window_height*0.075, true);	SELECTION_TEX = TextureManager::getInstance()->getTexture(inCurrentSelection(2) ? "data/textures/menu/OPT_SELECTION.tga": "data/textures/menu/OPT_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	OPT_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();

	E_QUAD.createQuad(inCurrentSelection(3) ? game->window_width*0.225 : game->window_width*0.2,
		game->window_height*0.7,
		inCurrentSelection(3) ? game->window_width*0.25 : game->window_width*0.2,
		game->window_height*0.075, true);	SELECTION_TEX = TextureManager::getInstance()->getTexture(inCurrentSelection(3) ? "data/textures/menu/EX_SELECTION.tga": "data/textures/menu/EX_NON_SELECTION.tga");
	SELECTION_TEX->bind();
	E_QUAD.render(GL_TRIANGLES);
	SELECTION_TEX->unbind();
}

void MenuState::update(double time_elapsed) {
	
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

	if (0) {
		BASS_ChannelStop(b_channel); // stop music
		game->bkg_music_playing = false;
	}
}

void MenuState::selectionUp()
{
	currentSelection--;
	if (currentSelection == -1)
		currentSelection = 3;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul2.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it

}

void MenuState::selectionDown()
{
	currentSelection++;
	if (currentSelection == 4)
		currentSelection = 0;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul2.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it

}

void MenuState::selectionChosen()
{
	if (game->effects_enabled)
	{
		s_sample = BASS_SampleLoad(false, "data/sounds/sel_menul.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
	}

	switch (currentSelection)
	{
	case 0:// play state
		SManager->changeCurrentState(PreplayState::getInstance(SManager));
		break;
	case 1: // how to play
		SManager->changeCurrentState(Howto::getInstance(SManager));
		break;
	case 2: // options state
		SManager->changeCurrentState(OptionsState::getInstance(SManager));
		break;
	case 3: // exit
		exit(1);
		break;
	}
}