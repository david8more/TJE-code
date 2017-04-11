#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "rendertotexture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "playstate.h"
#include "entity.h"
#include "world.h"

#include <cmath>

//globals

float angle = 0;
int i = 1;
bool update_ = true;
World * world = NULL;
bool shooting = false;
bool overused = false;
int shootingtime = 0;
int reparing_status = 0;

PlayState::PlayState(StateManager* SManager) : State(SManager) {}
PlayState::~PlayState() {}

PlayState* PlayState::getInstance(StateManager* SManager)
{
	static PlayState Instance(SManager);
	return &Instance;
}

void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	Game* game = Game::getInstance();

	switch (event.keysym.sym)
	{
	case SDLK_1:
		viewpos = Vector3(0, 5, -15);
		viewtarget = Vector3(0, 5, 0);
		break;
	case SDLK_2:
		viewpos = Vector3(0, 0.75, -1.5);
		viewtarget = Vector3(0, 0.5, 0);
		break;
	case SDLK_SPACE:
		shooting = true;
		break;
	}
}

void PlayState::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_SPACE:
		shooting = false;
		shootingtime = 0;
		BASS_ChannelStop(g_channel);
		break;
	case SDLK_r:
		reparing_status++;
		switch (reparing_status) {
		case 1:
			std::cout << "[o---------] 10% repaired!" << endl;
			break;
		case 5:
			std::cout << "[ooooo-----] 50% repaired!" << endl;
			break;
		case 10:
			std::cout << "[oooooooooo] 100% repaired!" << endl;
			break;
		}
		break;
	case SDLK_m:
		if (shooting) break;
		g_sample = BASS_SampleLoad(false, "data/sounds/missil.wav", 0L, 0, 1, 0);
		g_channel = BASS_SampleGetChannel(g_sample, false); // get a sample channel
		BASS_ChannelPlay(g_channel, false); // play it
		break;
	case SDLK_k:
		if (shooting) break;
		g_sample = BASS_SampleLoad(false, "data/sounds/explosion.wav", 0L, 0, 1, 0);
		g_channel = BASS_SampleGetChannel(g_sample, false); // get a sample channel
		BASS_ChannelPlay(g_channel, false); // play it
		break;
	}
}

void PlayState::init() {

	Game* game = Game::getInstance();
	//world = World::getInstance();
	world = new World();
	world->create();

	game->current_camera = new Camera();
	game->fixed_camera = new Camera();
	game->camera = new Camera();

	//create our camera
	game->camera->lookAt(Vector3(0, 507, 485), Vector3(0, 440, 1190), Vector3(0, 1, 0)); //position the camera and point to 0,0,0
	game->camera->setPerspective(70, game->window_width / (float)game->window_height, 0.1, 100000); //set the projection, we want to be perspective

	game->current_camera = game->camera;

	viewpos = Vector3(0, 5, -15);
	viewtarget = Vector3(0, 5, 0);

	game->current_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
}

void PlayState::onEnter()
{
	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

	cout << "$ Entering play state -- ..." << endl;
	Game* game = Game::getInstance();

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusi  ons using the Z buffer

	b_sample = BASS_SampleLoad(false, "data/sounds/music.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
	b_channel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
	//BASS_ChannelPlay(b_channel, false); // play it

	e_sample = BASS_SampleLoad(false, "data/sounds/plane.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
	e_channel = BASS_SampleGetChannel(e_sample, false); // get a sample channel
	BASS_ChannelPlay(e_channel, false); // play it

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse
}

void PlayState::render() {

	i++;

	Game* game = Game::getInstance();

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->current_camera->set();

	//Draw out world
	//drawGrid(500); //background grid

	world->root->render();

	//std::cout << "camera eye: " << game->current_camera->eye.x << ", " << game->current_camera->eye.y << ", " << game->current_camera->eye.z << endl;
	//std::cout << "camera center: " << game->camera->center.x << ", " << game->camera->center.y << ", " << game->camera->center.z << endl;

	glDisable(GL_BLEND);

	if (!overused && shootingtime > 20) {
		overused = true;
		reparing_status = 0;
		std::cout << "overused" << endl;
	}

	if (reparing_status > 9) overused = false;

	if (!shooting || overused) return;

	if (i % 10 != 0) return;

	g_sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
	g_channel = BASS_SampleGetChannel(g_sample, false); // get a sample channel
	BASS_ChannelPlay(g_channel, false); // play it
	shootingtime += 1;
}

void PlayState::update(double seconds_elapsed) {

	Game* game = Game::getInstance();

	double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

										  //mouse input to rotate the cam
	if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
	{
		world->playerAir->model.rotateLocal(game->mouse_delta.y * 0.005, Vector3(-1, 0, 0));
	}

	//async input to move the camera around
	if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
	if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(-1, 0, 0) * speed);
	if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(1, 0, 0) * speed);
	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, -1, 0) * speed);
	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 1, 0) * speed);
	if (game->keystate[SDL_SCANCODE_Q]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, 1) * speed);
	if (game->keystate[SDL_SCANCODE_E]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, -1) * speed);

	//to navigate with the mouse fixed in the middle
	if (game->mouse_locked)
	{
		int center_x = floor(game->window_width*0.5);
		int center_y = floor(game->window_height*0.5);

		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = center_x;
		game->mouse_position.y = center_y;
	}

	if (update_) {
		world->playerAir->model.traslateLocal(0, 0, speed * seconds_elapsed * 75);
		game->current_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
	}
	
	//rotar helice
	//world->helix->model.rotateLocal(90 * speed * seconds_elapsed * DEG2RAD, Vector3(0, 0, 1));

	angle += seconds_elapsed * 10;
}

void PlayState::onLeave(int fut_state) {
	BASS_ChannelStop(b_channel); // stop music
	BASS_ChannelStop(e_channel);
	BASS_ChannelStop(g_channel);
}

bool PlayState::isGameOver() {
	return false;
}

void PlayState::reset() {

}