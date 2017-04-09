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
#include <ctime>

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

int N = 250;

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

void MenuState::init() {

	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

	// Cargamos texturas de menú
	texture = new Texture();
	if (texture->load("data/textures/mainv1.tga"))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error in menu: texture has not been loaded" << endl;
		exit(1);
	}

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	// configuración inicial
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);

	// crear un box para la current selection
	sel_positions.resize(4);
	for (int i = 0; i < 4; ++i) {
		sel_positions[i].currents = i;
		sel_positions[i].posy = game->window_height * 0.34 + i * game->window_height * 0.1025;
		sel_positions[i].wid = game->window_width * 0.025;
	}

	sel_positions[0].posx = game->window_width * 0.445;
	sel_positions[1].posx = game->window_width * 0.39;
	sel_positions[2].posx = game->window_width * 0.427;
	sel_positions[3].posx = game->window_width * 0.457;

	particles.resize(N);
	positions.resize(N);
}

void MenuState::onEnter()
{
	cout << "$ Entering menu state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;

	if (game->bkg_music_playing != true && game->music_enabled) {
		b_sample = BASS_SampleLoad(false, "data/sounds/plane.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
		b_channel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
		//std::cout << "CHANEL" << b_channel << std::endl;
		BASS_ChannelPlay(b_channel, false); // play it
		game->bkg_music_playing = true;
	}

	srand(time(NULL));

	for (int i = 0; i < N; ++i) {
		particles[i] = new Mesh();
		float randomx = rand() % 1000 + 1;
		float randomy = rand() % 1000 + 1;
		positions[i].posx = game->window_width * randomx / 1000;
		positions[i].posy = game->window_height * randomy / 1000;
		particles[i]->createQuad(positions[i].posx, positions[i].posy, game->window_height*0.0025, game->window_height*0.0075, true);
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
	glDisable(GL_BLEND);

	// quad selection
	glColor3f(1.0, 1.0, 1.0);
	quadSelection.createQuad(sel_positions[currentSelection].posx, sel_positions[currentSelection].posy, sel_positions[currentSelection].wid, sel_positions[currentSelection].wid, true);
	quadSelection.render(GL_LINES);
	quadSelection.createQuad(sel_positions[currentSelection].posx + game->window_width * 0.0025, sel_positions[currentSelection].posy, sel_positions[currentSelection].wid, sel_positions[currentSelection].wid, true);
	quadSelection.render(GL_LINES);

	// particles
	glColor3f(0.2, 0.2, 0.4);

	for (int i = 0; i < N; i++) {
		particles[i]->render(GL_TRIANGLES);
	}

}

void MenuState::update(double time_elapsed) {
	
	for (int i = 0; i < N; ++i) {
		float newPos = positions[i].posy + 8;
		if (newPos >= game->window_height) newPos = 0;
		particles[i]->createQuad(positions[i].posx, newPos, game->window_height*0.0025, game->window_height*0.0075, true);
		positions[i].posy = newPos;
	}
}

void MenuState::onLeave(int fut_state) {

	// solo paramos si vamos al play state
	if (0) {
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