#include "state.h"
#include "playstate.h"
#include "endingstate.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../bass.h"
#include "../entity.h"
#include "../world.h"
#include "../bulletmanager.h"
#include "../playercontroller.h"

#include <cmath>

#define DEBUG 0

//globals
BulletManager* bManager = NULL;
World * world = NULL; 

Mesh debug_mesh;

PlayState::PlayState(StateManager* SManager) : State(SManager)
{
	inZoom = false;
}

PlayState::~PlayState() {}

void PlayState::init() {

	game = Game::getInstance();
	bManager = BulletManager::getInstance();
	world = World::getInstance();
	world->create();

	PlayerController::getInstance()->setPlayer(world->playerAir);

	// posicion y direccion de la vista seleccionada
	viewpos = Vector3(0, 5, -15);
	viewtarget = Vector3(0, 5, 0);

	//create our camera
	game->free_camera = new Camera(); //our global camera
	game->fixed_camera = new Camera();

	//game->fixed_camera->lookAt(world->playerAir->model * viewpos, world->playerAir->model * viewtarget, world->playerAir->model.rotateVector(Vector3(0, 1, 0)));
	game->fixed_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 7.5f, 50000.f);

	game->free_camera->lookAt(Vector3(2000, 0, -2000), Vector3(0,0, 0), Vector3(0, 1, 0));
	game->free_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1, 100000.f);

	game->current_camera = DEBUG ? game->free_camera : game->fixed_camera;

	// fill views struct
	// METER ZOOM DEPENDIENDO DEL AVION!!
	// sTransZoomCreator(view, planeModel, quantityToTranslate);
	sTransZoomCreator(FULLVIEW, SPITFIRE, 7.5f);
	sTransZoomCreator(CABINEVIEW, SPITFIRE, 0.5f);

	sTransZoomCreator(FULLVIEW, P38, 7.5f);
	sTransZoomCreator(CABINEVIEW, P38, 0.65f);

	sTransZoomCreator(FULLVIEW, WILDCAT, 7.5f);
	sTransZoomCreator(CABINEVIEW, WILDCAT, 0.75f);

	sTransZoomCreator(FULLVIEW, BOMBER, 7.5f);
	sTransZoomCreator(CABINEVIEW, BOMBER, 0.75f);

	// collision models

	for (int i = 0; i < EntityCollider::static_colliders.size(); i++) {
		EntityCollider * current_collider = EntityCollider::static_colliders[i];
		Mesh::Get(current_collider->mesh.c_str())->setCollisionModel();
	}

	// HUD
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, 25, 25);

	crosshair_tex = "data/textures/target_32.tga";
}

void PlayState::onEnter()
{
	cout << "$ Entering play state -- ..." << endl;
	
	player = World::getInstance()->playerAir;
	// views things
	current_view = 0;

	//set the clear color (the background color)
	glClearColor(0.5, 0.7, 0.8, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusi  ons using the Z buffer

	//m60 init
	player->timer = player->shootingtime = 0;
	player->cadenceCounter = 0;
	player->shooting = player->overused = false;

	// Sounds
	if (game->music_enabled) {
		b_sample = BASS_SampleLoad(false, "data/sounds/music.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
		hSampleChannel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
		BASS_ChannelSetAttribute(hSampleChannel, BASS_ATTRIB_VOL, game->BCK_VOL);
		BASS_ChannelPlay(hSampleChannel, false); // play it
	}

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse

}

void PlayState::render() {

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->current_camera->set();
	
	glDisable(GL_DEPTH_TEST);
	world->sky->model.setIdentity();
	world->sky->model.traslate(game->current_camera->eye.x, game->current_camera->eye.y, game->current_camera->eye.z);
	world->sky->render(game->current_camera);
	glEnable(GL_DEPTH_TEST);

	Entity * sea = Entity::getEntity("sea");
	sea->model.setIdentity();
	sea->model.traslate(game->current_camera->eye.x, -10, game->current_camera->eye.z);
	sea->render(game->current_camera);

	world->root->render(game->current_camera);

	//
	if (debug_mesh.vertices.size()) {
		debug_mesh.render(GL_POINTS);
	}
	//
	bManager->render();
	renderHUD();
}

void PlayState::update(double seconds_elapsed) {

	PlayerController* player_controller = PlayerController::getInstance();

	// ********************************************************************

	if (game->current_camera == game->free_camera)
	{	
		player_controller->updateCamera(game->current_camera, seconds_elapsed);
		return;
	}
	else
	{	
		player_controller->update(seconds_elapsed);
	}

	// update bullets and more
	world->root->update(seconds_elapsed);

	// interpolate current and previous camera
	Vector3 eye = player->model * viewpos;

	// evitamos que se mueva en la vista de cabina
	if(current_view == FULLVIEW)
		eye = game->current_camera->eye * 0.85 + eye * 0.15;
	
	game->fixed_camera->lookAt(eye, player->model * viewtarget, player->model.rotateVector(Vector3(0, 1, 0)));

	// COLISIONES

	Vector3 ray_origin;
	Vector3 ray_dir;

	if (DEBUG) {
		//ray_origin = game->current_camera->eye;
		//ray_dir = (game->current_camera->center - ray_origin).normalize();
	}
	else {
		ray_origin = game->current_camera->eye;
		ray_dir = (game->current_camera->center - ray_origin).normalize();
	}

	Vector3 coll;

	if (EntityCollider::testRayWithAll(ray_origin, ray_dir, 10000000, coll)) {
		//std::cout << coll.x << coll.y << coll.z;
		debug_mesh.vertices.push_back(coll);
		debug_mesh.colors.push_back(Vector4(1, 0, 0, 0));
	}

	// borrar pendientes
	Entity::destroy_entities();

	// comprobar si es fin del juego
	if (world->isGameOver())
	{
		std::cout << "Game has finished!" << std::endl;
		exit(1);
		//SManager->changeCurrentState(EndingState::getInstance(SManager));
	}

}

void PlayState::renderHUD() {

	// RENDER HUD ****************************************************************

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	Texture::Get(crosshair_tex.c_str())->bind();
	quad.render(GL_TRIANGLES);
	Texture::Get(crosshair_tex.c_str())->unbind();
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glDisable(GL_BLEND);

	// M60 overusing HUD

	std::stringstream ss;

	ss << "[";
	for (int i = 0; i < player->shootingtime; i += 3) {
		ss << "|";
	}
	ss << "]";

	float pRcolor = ss.str().size() / (float)13;
	Vector3 v(pRcolor, 1.f - pRcolor, 0.f);

	drawText(game->window_width*0.8, game->window_height*0.9, ss.str(), v, 3.0);
	int percent_used = (float)player->shootingtime / 30 * 100;
	if (percent_used > 100.0) percent_used = 100.0;
	ss.str("");
	ss << percent_used << "%";
	drawText(game->window_width*0.8, game->window_height*0.95, ss.str(), v, 2.0); // % engine !!!
	if (player->overused) drawText(game->window_width*0.1, game->window_height*0.75, "** ALERT: ENGINE OVERHEAT ** COOLING SYSTEM...", Vector3(1, 0, 0), 3.0);

	// MISSILES

	ss.str("");
	ss << "Torpedos left: " << world->playerAir->torpedosLeft;
	drawText(game->window_width*0.75, game->window_height*0.1, ss.str(), Vector3(1, 0, 0), 2.0); // % engine !!!

	// vidas enemigas

	
	if (EntityCollider::static_colliders.size())
	{
		for (int i = 0; i < EntityCollider::static_colliders.size(); i++)
		{
			ss.str("");
			ss << EntityCollider::static_colliders[i]->life;
			drawText(game->window_width*0.1, game->window_height*0.1*(i+1), ss.str(), Vector3(1, 0, 0), 3.0);
		}

		
	}

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// FINISHED RENDER INTERFACE ****************************************************************
}

void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_0:
		game->start = true;
		player->engineOnOff();
		e_sample = BASS_SampleLoad(false, "data/sounds/plane.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
		e_channel = BASS_SampleGetChannel(e_sample, false); // get a sample channel
		BASS_ChannelPlay(e_channel, false); // play it
		break;
	case SDLK_1: // full plane view or cabine view
		current_view = current_view == FULLVIEW ? CABINEVIEW : FULLVIEW;
		setView();
		break;
	case SDLK_3:
		game->current_camera = game->current_camera == game->fixed_camera ? game->free_camera : game->fixed_camera;
		break;
	case SDLK_4:
		Shader::ReloadAll();
		break;
	case SDLK_5:
		PlayerController::getInstance()->current_controller = CONTROLLER_MODE_KEYBOARD;
		break;
	case SDLK_p: // PARAR MOTOR
		player->engineOnOff();
	
		if (player->engine)
		{
			PlayerController::getInstance()->arranque = 0;
			e_sample = BASS_SampleLoad(false, "data/sounds/plane.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
			e_channel = BASS_SampleGetChannel(e_sample, false); // get a sample channel
			BASS_ChannelPlay(e_channel, false); // play it
		}
		else {
			BASS_ChannelStop(e_channel);
		}
		break;
	case SDLK_SPACE:
		world->playerAir->shooting = true;
		break;
	
	}
}

void PlayState::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_SPACE:
		world->playerAir->shooting = false;
		if (!player->overused)
			player->shootingtime = 0;
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
	}
}

void PlayState::setZoom()
{
	// si es el model 2, irá a la posición 2*2 = 4(full plane). Si current view = 0, 4+4 = 0(full plane)
	// si cv = 1, 4+1 = 5 que es de la cabina
	if (inZoom)
		viewpos.z += vTranslations[world->worldInfo.playerModel * 2 + current_view].qnt;
	else
		setView();
}

void PlayState::onLeave(int fut_state) {
	BASS_ChannelStop(hSampleChannel); // stop music
	BASS_ChannelStop(e_channel);
}

void PlayState::setView() {

	switch (current_view)
	{
	case FULLVIEW:

		game->current_camera->near_plane = 7.5f;
		game->current_camera->far_plane = 50000.f;

		if (world->worldInfo.playerModel == SPITFIRE) {
			world->playerAir->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		}
		viewpos = Vector3(0, 5, -15);
		viewtarget = Vector3(0, 5, 10.f);
		break;

	case CABINEVIEW:

		game->current_camera->near_plane = 0.1f;
		game->current_camera->far_plane = 50000.f;
		
		if (world->worldInfo.playerModel == SPITFIRE) {
			viewpos = Vector3(0.f, 0.7f, -1.5f);
			viewtarget = Vector3(0.f, 0.5f, 10.f);
			world->playerAir->set("spitfire_cabina.ASE", "data/textures/spitfire_cabina_alpha.tga", "simple");

		}
		else if (world->worldInfo.playerModel == P38) {
			viewpos = Vector3(-0.03f, 0.75f, 0.752f);
			viewtarget = Vector3(0.f, 0.5f, 10.f);
		} 
		else if (world->worldInfo.playerModel == WILDCAT) {
			viewpos = Vector3(0.f, 1.f, -0.5f);
			viewtarget = Vector3(0.f, 1.f, 10.f);
		}
		else if (world->worldInfo.playerModel == BOMBER) {
			viewpos = Vector3(0.5f, 0.f, 2.5f);
			viewtarget = Vector3(0.f, 1.f, 10.f);
		}

		break;
	}
}