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

#define N 100 // lluvia

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

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
	texture = TextureManager::getInstance()->getTexture("data/textures/mainv1.tga");

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
		//std::cout << "CHANEL" << b_channel << std::endl;
		BASS_ChannelPlay(b_channel, false); // play it
		game->bkg_music_playing = true;
	}

	srand(time(NULL));

	vParticles.resize(N);

	for (int i = 0; i < N; ++i) {
		float randomx = rand() % 1000 + 1;
		float randomy = rand() % 1000 + 1;
		vParticles[i].posx = game->window_width * randomx / 1000;
		vParticles[i].posy = game->window_height * randomy / 1000;
		mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));
		mParticles.colors.push_back(Vector4(0, 0, 1, 1));
	}

}

void MenuState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	
	for (int i = 0; i < N; ++i) {
		mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));
		mParticles.colors.push_back(Vector4(0.25, 0.25, 0.75, 1.0));
	}

	glPointSize(2);
	mParticles.render(GL_POINTS); // renderizar de una vez toda la lluvia

}

void MenuState::update(double time_elapsed) {
	
	mParticles.clear();

	for (int i = 0; i < N; ++i) {
		float newPos = vParticles[i].posy + 450 * time_elapsed;
		if (newPos >= game->window_height) newPos = newPos - game->window_height;
		vParticles[i].posy = newPos;
		
		vParticles[i].posx += 100 * time_elapsed;
		if (vParticles[i].posx >= game->window_width) vParticles[i].posx -= game->window_width;
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