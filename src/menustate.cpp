#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "rendertotexture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "menustate.h"
#include "playstate.h"
#include "howto.h"
#include <cmath>

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

typedef struct {
	int currents;
	int posx;
	int posy;
} sPositions;

std::vector<sPositions> sel_positions;

MenuState* MenuState::getInstance(StateManager* SManager)
{
	static MenuState Instance(SManager);
	return &Instance;
}

void MenuState::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_DOWN:
		selectionDown();
		break;
	case SDLK_UP:
		selectionUp();
		break;
	case SDLK_RETURN:
		selectionChosen();
		break;
	}
}

void MenuState::init() {

	texture = new Texture();
	if (texture->load("data/textures/mainmenu.tga"))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error in menu: texture has not been loaded" << endl;
		exit(1);
	}

	game = Game::getInstance();
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);

	// crear un box para la current selection
	sel_positions.resize(4);
	for (int i = 0; i < 4; ++i) {
		sel_positions[i].currents = i;
		sel_positions[i].posx = game->window_width * 0.53;
		sel_positions[i].posy = game->window_height * 0.41 + i * 77;
	}
}

void MenuState::onEnter()
{
	cout << "$ Entering menu state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;

	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

	if (game->bkg_music_playing != true && game->music_enabled) {
		b_sample = BASS_SampleLoad(false, "data/sounds/bkg_menu.wav", 0L, 0, 1, 0);
		b_channel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
		//cout << "CHANEL" << b_channel << endl;

		BASS_ChannelPlay(b_channel, false); // play it
		game->bkg_music_playing = true;
	}
}

void MenuState::render() {

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	glEnable(GL_BLEND);
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();

	// quad selection
	glColor3f(1.0, 1.0, 1.0);
	quadSelection.createBox(sel_positions[currentSelection].posx, sel_positions[currentSelection].posy, game->window_width * 0.35, game->window_height * 0.115, true);
	quadSelection.render(GL_LINES);
	quadSelection.createBox(sel_positions[currentSelection].posx + 5, sel_positions[currentSelection].posy + 5, game->window_width * 0.35, game->window_height * 0.115, true);
	quadSelection.render(GL_LINES);

	glDisable(GL_BLEND);
}

void MenuState::update(double time_elapsed) {

}

void MenuState::onLeave(int fut_state) {

	if (fut_state == 1) {
		BASS_ChannelStop(b_channel); // stop music
		//cout << "CHANEL" << b_channel << endl;
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

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul.wav", 0L, 0, 1, 0);
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

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it

}

void MenuState::selectionChosen()
{
	cout << currentSelection << endl;
	if (game->effects_enabled)
	{
		s_sample = BASS_SampleLoad(false, "data/sounds/sel_menul.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
	}

	switch (currentSelection)
	{
	case 0:// play state
		SManager->changeCurrentState(PlayState::getInstance(SManager));
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