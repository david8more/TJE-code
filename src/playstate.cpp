#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "playstate.h"
#include "entity.h"
#include "world.h"
#include "bulletmanager.h"

#include <cmath>

//globals

BulletManager* bManager = new BulletManager();

float angle = 0;
int i = 1;
bool update_ = true;
World * world = NULL;
bool shooting = false;
bool overused = false;
int shootingtime = 0;
float timer = 0;

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
		if (!overused) shootingtime = 0;
		BASS_ChannelStop(g_channel);
		break;
	case SDLK_m:
		if (shooting) break;
		world->playerAir->missileShoot();
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
	game->camera = new Camera();

	//create our camera
	game->camera->lookAt(Vector3(0, 507, 485), Vector3(0, 440, 1190), Vector3(0, 1, 0)); //position the camera and point to 0,0,0
	game->camera->setPerspective(70, game->window_width / (float)game->window_height, 0.1, 100000); //set the projection, we want to be perspective

	game->current_camera = game->camera;

	viewpos = Vector3(0, 5, -15);
	viewtarget = Vector3(0, 5, 0);

	game->current_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));

	// GUI
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width * 0.1, game->window_height*0.1, true);

	crosshair_tex = new Texture();
	if (crosshair_tex->load("data/textures/crosshair.tga"))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error in playstate: texture(CROSSHAIR) has not been loaded" << endl;
		exit(1);
	}
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
	BASS_ChannelPlay(b_channel, false); // play it

	e_sample = BASS_SampleLoad(false, "data/sounds/plane.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
	e_channel = BASS_SampleGetChannel(e_sample, false); // get a sample channel
	BASS_ChannelPlay(e_channel, false); // play it

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse
}

void PlayState::render() {

	i++;

	Game* game = Game::getInstance();
	//std::cout << game->mouse_position.x << " - " << game->mouse_position.y << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->current_camera->set();

	glEnable(GL_BLEND);

	world->root->render();
	bManager->render();

	// RENDER INTERFACE

	/*glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	crosshair_tex->bind();
	quad.render(GL_TRIANGLES);
	crosshair_tex->unbind();*/

	std::stringstream ss;
	Vector3 v;

	ss << "[";
	for (int i = 0; i < shootingtime; i += 3) {
		ss << "|";
	}
	ss << "]";

	if(ss.str().size() > 11) v = Vector3(1, 0, 0);
	else if(ss.str().size() > 8) v = Vector3(0.95, 0.6, 0.25);
	else if (ss.str().size() > 4) v = Vector3(1, 1, 0);
	else v = Vector3(0, 1, 0);

	drawText(game->window_width*0.8, game->window_height*0.9, ss.str(), v, 3.0);
	ss.str("");
	int percent_used = (float)shootingtime / 30 * 100;
	if (percent_used > 100.0) percent_used = 100.0;
	ss << percent_used;
	ss << "%";
	drawText(game->window_width*0.8, game->window_height*0.95, ss.str(), v, 2.0); // % engine !!!
	if(overused) drawText(game->window_width*0.1, game->window_height*0.75, "ALERT: ENGINE OVERUSED. COOLING SYSTEM...", Vector3(1, 0, 0), 3.0);
	
	glDisable(GL_BLEND);

	//SDL_GL_SwapWindow(game->window);
	
	// **** FINISHED RENDER INTERFACE

	if (!overused && shootingtime > 30) {
		overused = true;
		std::cout << "overused" << endl;
	}

	if (!shooting || overused || i % 10 != 0) return;

	g_sample = BASS_SampleLoad(false, "data/sounds/shot.wav", 0L, 0, 1, 0);
	g_channel = BASS_SampleGetChannel(g_sample, false); // get a sample channel
	BASS_ChannelPlay(g_channel, false); // play it
	shootingtime += 1;

	world->playerAir->m60Shoot();
	
}

void PlayState::update(double seconds_elapsed) {

	// overusing

	if (overused) {
		timer += seconds_elapsed;
	}

	if (timer > 5) {
		timer = 0;
		overused = false;
		shootingtime = 0;
	}

	Game* game = Game::getInstance();

	double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

	//mouse input to rotate the cam
	if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
	{
		world->playerAir->model.rotateLocal(game->mouse_delta.y * 0.005, Vector3(1, 0, 0));
	}

	//async input to move the camera around
	if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 15; //move faster with left shift
	if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(-1, 0, 0) * speed);
	if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(1, 0, 0) * speed);
	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) //world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, -1, 0) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, 1) * speed);
	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) //world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 1, 0) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, -1) * speed);
	if (game->keystate[SDL_SCANCODE_Q]) {
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, 1) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, -1, 0) * speed);
	}
	if (game->keystate[SDL_SCANCODE_E]) {
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, -1) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 1, 0) * speed);
	}
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
		world->playerAir->model.traslateLocal(0, 0, speed * seconds_elapsed * 100);
		game->current_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
		world->playerAir->update(seconds_elapsed);
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