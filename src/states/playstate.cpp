#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "state.h"
#include "../bass.h"
#include "playstate.h"
#include "../entity.h"
#include "../world.h"
#include "../bulletmanager.h"
#include "endingstate.h"

#include <cmath>

#define DEBUG 0

//globals
bool inZoom = false;
BulletManager* bManager = NULL;
World * world = NULL;

PlayState::PlayState(StateManager* SManager) : State(SManager) {}
PlayState::~PlayState() {}

void PlayState::init() {

	Game* game = Game::getInstance();
	bManager = BulletManager::getInstance();
	world = World::getInstance();
	world->create();

	// posicion y direccion de la vista seleccionada
	viewpos = Vector3(0, 5, -15);
	viewtarget = Vector3(0, 5, 0);

	//create our camera
	game->free_camera = new Camera(); //our global camera
	game->fixed_camera = new Camera();

	game->fixed_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
	game->fixed_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1, 100000.f);

	game->free_camera->lookAt(Vector3(0, 500, 500), world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
	game->free_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1, 100000.f);

	game->current_camera = DEBUG ? game->free_camera : game->fixed_camera;

	// fill views struct
	// remember: sTransZoomCreator(view, planeModel, quantityToTranslate);
	sTransZoomCreator(0, 0, 7.5f);
	sTransZoomCreator(1, 0, 0.5f);
	sTransZoomCreator(0, 1, 7.5f);
	sTransZoomCreator(1, 1, 0.5f);
	sTransZoomCreator(0, 2, 7.5f);
	sTransZoomCreator(0, 2, 0.45f);
	sTransZoomCreator(1, 3, 7.5f); // falta probar estos dos
	sTransZoomCreator(1, 3, 0.5f); // falta probar estos dos

	// collision models

	for (int i = 0; i < world->collision_enemies.size(); i++) {
		EntityEnemy * current_enemy = world->collision_enemies[i];
		current_enemy->mesh->setCollisionModel();
	}

	// HUD
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, 50, 50);

	crosshair_tex = Texture::Get("data/textures/crosshair.tga");
}

void PlayState::onEnter()
{
	cout << "$ Entering play state -- ..." << endl;
	Game* game = Game::getInstance();
	
	// views things
	current_view = 0;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusi  ons using the Z buffer

	//m60 init
	timer = shootingtime = 0;
	cadencia = 0;
	shooting = overused = false;

	// Sounds
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

	Game* game = Game::getInstance();

	//set the clear color (the background color)
	glClearColor(0.05, 0.14, 0.25, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->current_camera->set();
	
	glDisable(GL_DEPTH_TEST);
	world->sky->model.setIdentity();
	world->sky->model.traslate(game->current_camera->eye.x, game->current_camera->eye.y, game->current_camera->eye.z);
	world->sky->render(game->current_camera);
	glEnable(GL_DEPTH_TEST);

	world->root->render(game->current_camera);
	bManager->render();
	renderHUD();
}

bool a = false;

void PlayState::update(double seconds_elapsed) {

	// overusing y cadencia

	if (!overused && shootingtime > 30) overused = true;

	if (shooting && !overused && cadencia <= 0){
		shootingtime++;
		world->playerAir->m60Shoot();
		cadencia = world->playerAir->cadence;
	}

	if (cadencia > 0) cadencia -= seconds_elapsed * 100;

	// cooling system timer
	if (overused) timer += seconds_elapsed;

	if (timer > 5) {
		timer = shootingtime = 0;
		overused = false;
	}

	// ********************************************************************

	Game* game = Game::getInstance();

	double speed = seconds_elapsed * 20; //the speed is defined by the seconds_elapsed so it goes constant
	bool mod = false;

	if (DEBUG) {
		if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 150;
		if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) game->current_camera->move(Vector3(0.f, 0.f, 1.f) * speed);
		if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) game->current_camera->move(Vector3(0.f, 0.f, -1.f) * speed);
		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) game->current_camera->move(Vector3(1.f, 0.f, 0.f) * speed);
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) game->current_camera->move(Vector3(-1.f, 0.f, 0.f) * speed);

		if (game->mouse_locked || (game->mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
		{
			game->current_camera->rotate(game->mouse_delta.x * 0.005f, Vector3(0.f, -1.f, 0.f));
			game->current_camera->rotate(game->mouse_delta.y * 0.005f, game->current_camera->getLocalVector(Vector3(-1.f, 0.f, 0.f)));
		}
	}
	else {
		//mouse input to rotate the cam
		if (game->mouse_locked || (game->mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
		{
			if (game->current_camera == game->fixed_camera) {
				world->playerAir->model.rotateLocal(game->mouse_delta.x * 0.005, Vector3(0, -1, 0));
				world->playerAir->model.rotateLocal(game->mouse_delta.y * 0.005, Vector3(1, 0, 0));
			}
			else {
				mod = true;
				
				game->free_camera->rotate(game->mouse_delta.x * -0.005f, viewtarget);
				game->free_camera->center.y = (world->playerAir->model * viewtarget).y;
				game->free_camera->center.z = (world->playerAir->model * viewtarget).z;
				game->free_camera->eye = world->playerAir->model * viewpos;
			}
		}

		//async input to move the camera around
		if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 150; //move faster with left shift
		if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(-1.f, 0.f, 0.f) * speed);
		if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(1.f, 0.f, 0.f) * speed);
		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0.f, 0.f, 1.f) * speed);
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0.f, 0.f, -1.f) * speed);
		if (game->keystate[SDL_SCANCODE_Q]) {
			world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, 1) * speed);
			world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, -1, 0) * speed);
		}
		if (game->keystate[SDL_SCANCODE_E]) {
			world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, -1) * speed);
			world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 1, 0) * speed);
		}
		if (game->keystate[SDL_SCANCODE_T]) {
			
		}
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

	// update part

	world->playerAir->update(seconds_elapsed);
	
	if (DEBUG) return;
	
	world->playerAir->model.traslateLocal(0, 0, speed * seconds_elapsed * 10);
	game->fixed_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
	if (!mod) game->free_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));

	// comprobar si es fin del juego
	if (world->isGameOver()) SManager->changeCurrentState(EndingState::getInstance(SManager));
}

void PlayState::renderHUD() {

	Game* game = Game::getInstance();

	// RENDER HUD ****************************************************************

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	crosshair_tex->bind();
	quad.render(GL_TRIANGLES);
	crosshair_tex->unbind();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glDisable(GL_BLEND);

	// M60 overusing HUD

	std::stringstream ss;

	ss << "[";
	for (int i = 0; i < shootingtime; i += 3) {
		ss << "|";
	}
	ss << "]";

	float pRcolor = ss.str().size() / (float)13;
	Vector3 v(pRcolor, 1.f - pRcolor, 0.f);

	drawText(game->window_width*0.8, game->window_height*0.9, ss.str(), v, 3.0);
	int percent_used = (float)shootingtime / 30 * 100;
	if (percent_used > 100.0) percent_used = 100.0;
	ss.str("");
	ss << percent_used << "%";
	drawText(game->window_width*0.8, game->window_height*0.95, ss.str(), v, 2.0); // % engine !!!
	if (overused) drawText(game->window_width*0.1, game->window_height*0.75, "ALERT: ENGINE OVERUSED. COOLING SYSTEM...", Vector3(1, 0, 0), 3.0);

	// MISSILES

	ss.str("");
	ss << "Torpedos left: " << world->playerAir->torpedosLeft;
	drawText(game->window_width*0.75, game->window_height*0.1, ss.str(), Vector3(1, 0, 0), 2.0); // % engine !!!

	// vidas enemigas

	ss.str("");
	ss << world->collision_enemies[0]->life;
	drawText(game->window_width*0.1, game->window_height*0.1, ss.str(), Vector3(1, 0, 0), 3.0);
	ss.str("");
	ss << world->collision_enemies[1]->life;
	drawText(game->window_width*0.1, game->window_height*0.2, ss.str(), Vector3(1, 0, 0), 3.0);
	ss.str("");
	ss << world->collision_enemies[2]->life;
	drawText(game->window_width*0.1, game->window_height*0.3, ss.str(), Vector3(1, 0, 0), 3.0);

	// FINISHED RENDER INTERFACE ****************************************************************
}
void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	Game* game = Game::getInstance();

	switch (event.keysym.sym)
	{
	case SDLK_1: // full plane view
		current_view = 0;
		setView();
		break;
	case SDLK_2: // cabine view
		current_view = 1;
		setView();
		break;
	case SDLK_3:
		game->current_camera = game->current_camera == game->fixed_camera ? game->free_camera : game->fixed_camera;
		break;
	case SDLK_SPACE:
		shooting = true;
		break;
	
	}
}

void PlayState::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_SPACE:
		shooting = false;
		if (!overused) shootingtime = 0;
		break;
	case SDLK_t:
		world->playerAir->torpedoShoot();
		break;
	}
}

void PlayState::onMouseButton(SDL_MouseButtonEvent event) {

	if (event.button == SDL_BUTTON_RIGHT) //right mouse
	{
		inZoom = !inZoom;
		
		// si es el model 2, irá a la posición 2*2 = 4(full plane). Si current view = 0, 4+4 = 0(full plane)
		// si cv = 1, 4+1 = 5 que es de la cabina
		if (inZoom) viewpos.z += vTranslations[world->worldInfo.playerModel * 2 + current_view].qnt;	
		else setView();
	}
}


void PlayState::onLeave(int fut_state) {
	BASS_ChannelStop(b_channel); // stop music
	BASS_ChannelStop(e_channel);
}

void PlayState::setView() {
	switch (current_view)
	{
	case FULLVIEW:
		if (world->worldInfo.playerModel == SPITFIRE) {
			world->playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		}
		viewpos = Vector3(0, 5, -15);
		viewtarget = Vector3(0, 5, 10.f);
		break;

	case CABINEVIEW:
		
		// arreglar la vista del p38!!!!!
		if (world->worldInfo.playerModel == SPITFIRE) {
			viewpos = Vector3(0.f, 0.7f, -1.45f);
			viewtarget = Vector3(0.f, 0.5f, 10.f);
			world->playerAir->set("spitfire_cabina.ASE", "data/textures/spitfire_cabina_alpha.tga", "simple");

		}
		else if (world->worldInfo.playerModel == P38) {
			viewpos = Vector3(0.f, 0.75f, -1.5f); viewtarget = Vector3(0.f, 0.5f, 0.f);
		} 
		else if (world->worldInfo.playerModel == WILDCAT) {
			viewpos = Vector3(0.f, 1.f, -0.5f); viewtarget = Vector3(0.f, 1.f, 0.f);
		}
		else if (world->worldInfo.playerModel == BOMBER) {
			viewpos = Vector3(0.5f, 0.f, 2.5f); viewtarget = Vector3(0.f, 1.f, 10.f);
		}

		break;
	}
}