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

#define N 50 // lluvia
#define PARTICLES 1
float x = 200.f; float y = 200.f;
bool loaded = false;

MenuState::MenuState(StateManager* SManager) : State(SManager) {}
MenuState::~MenuState() {}

void MenuState::init() {

	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

	// Cargamos texturas de menú
	texture = TextureManager::getInstance()->getTexture("data/textures/mainv1.tga");
	smokeTexture = TextureManager::getInstance()->getTexture("data/textures/smoke_alpha.tga");
	loadingTexture = TextureManager::getInstance()->getTexture("data/textures/loading.tga");

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	// configuración inicial
	loadingQuad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
	backgroundQuad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);

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

	if (!PARTICLES) return;
	
	vParticles.resize(N);

	for (int i = 0; i < N; ++i) {
		float randomx = rand() % 1000 + 1;
		float randomy = rand() % 1000 + 1;
		vParticles[i].posx = game->window_width * randomx / 1000;
		vParticles[i].posy = game->window_height * randomy / 1000;
		mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));
		mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));
		mParticles.colors.push_back(Vector4(1.0, 1.0, 1.0, 1.0));
		mParticles.colors.push_back(Vector4(1.0, 1.0, 1.0, 1.0));
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
	glColor4f(1.f, 1.f, 1.f, 1.f);
	
	// fondo
	texture->bind();
	backgroundQuad.render(GL_TRIANGLES);
	texture->unbind();

	// smoke
	quad.createQuad(x, y, game->window_width, game->window_height, true);
	quad2.createQuad(x + 100.f, y + 150.f, game->window_width, game->window_height, true);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	smokeTexture->bind();
	quad.render(GL_TRIANGLES);
	quad2.render(GL_TRIANGLES);
	smokeTexture->unbind();
	glDisable(GL_BLEND);


	// quad selection
	quadSelection.createQuad(sel_positions[currentSelection].posx, sel_positions[currentSelection].posy, sel_positions[currentSelection].wid, sel_positions[currentSelection].wid, true);
	quadSelection.render(GL_LINES);
	quadSelection.createQuad(sel_positions[currentSelection].posx + game->window_width * 0.0025, sel_positions[currentSelection].posy, sel_positions[currentSelection].wid, sel_positions[currentSelection].wid, true);
	quadSelection.render(GL_LINES);

	// particles

	if (PARTICLES) mParticles.render(GL_LINES); // renderizar de una vez toda la lluvia
	
	// loading screen
	if (loaded) return;

	loadingTexture->bind();
	loadingQuad.render(GL_TRIANGLES);
	loadingTexture->unbind();
	drawText(game->window_width*0.35, game->window_height*0.75, "[Press any key to continue]", Vector3(1.f, 1.f, 1.f), game->window_width*0.002);

}

void MenuState::update(double time_elapsed) {
	
	if (!PARTICLES) return;

	mParticles.clear();

	for (int i = 0; i < N; ++i) {

		float newPosX = vParticles[i].posx + 75 * time_elapsed;
		float newPosY = vParticles[i].posy + 250 * time_elapsed;
		if (newPosY >= game->window_height) {
			newPosY = newPosY - game->window_height;
			if (newPosX >= game->window_width) {
				mParticles.vertices.push_back(Vector3(0, 0, 0));
				vParticles[i].posx = newPosX - game->window_width;
				vParticles[i].posy = newPosY;
			}
			else {
				mParticles.vertices.push_back(Vector3(vParticles[i].posx, 0, 0));
				vParticles[i].posx = newPosX;
				vParticles[i].posy = newPosY;
			}
		}
		else {
			if (newPosX >= game->window_width) {
				mParticles.vertices.push_back(Vector3(0, vParticles[i].posy, 0));
				vParticles[i].posx = newPosX - game->window_width;
				vParticles[i].posy = newPosY;
			}
			else {
				mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));
				vParticles[i].posx = newPosX;
				vParticles[i].posy = newPosY;
				
			}
		}

		mParticles.vertices.push_back(Vector3(vParticles[i].posx, vParticles[i].posy, 0));

		mParticles.colors.push_back(Vector4(0.25, 0.25, 0.75, 1.0));
		mParticles.colors.push_back(Vector4(0.25, 0.25, 0.75, 1.0));
	}
}

void MenuState::onKeyPressed(SDL_KeyboardEvent event)
{
	if (!loaded) {
		loaded = !loaded;
		return;
	}

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