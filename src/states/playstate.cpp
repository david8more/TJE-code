#include "state.h"
#include "playstate.h"
#include "endingstate.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "../entity.h"
#include "../world.h"
#include "../bulletmanager.h"
#include "../explosion.h"
#include "../playercontroller.h"
#include "../rendertotexture.h"
#include "../soundmanager.h"

#include <cmath>

#define DEBUG 0

//globals
BulletManager* bManager = NULL;
World * world = NULL; 
Mesh debug_mesh;
bool pause = false;

int controlIA = 0;

RenderToTexture * rt = NULL;

PlayState::PlayState(StateManager* SManager) : State(SManager)
{
	inZoom = false;
}

PlayState::~PlayState() {}

void PlayState::init()
{
	std::cout << "init playstate" << std::endl;

	bManager = BulletManager::getInstance();
	game = Game::getInstance();
	world = World::getInstance();
	world->create();

	// posicion y direccion de la vista seleccionada
	viewpos = Vector3(0.f, 6.f, -6.5f);
	viewtarget = Vector3(0.f, 0.f, 100.f);

	//create our camera
	game->free_camera = new Camera(); //our global camera
	game->fixed_camera = new Camera();
	game->fixed_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 7.5f, 50000.f);

	game->free_camera->lookAt(Vector3(135, 50, -410), Vector3(0,0, 0), Vector3(0, 1, 0));
	game->free_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 1.0, 25000.f);

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

	// set collision models

	for (int i = 0; i < EntityCollider::static_colliders.size(); i++)
	{
		EntityCollider * current_collider = EntityCollider::static_colliders[i];
		Mesh::Get(current_collider->mesh.c_str())->setCollisionModel();
	}

	// HUD
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, 50, 50, true);

	crosshair_tex = "data/textures/crosshair.tga";

	rt = new RenderToTexture();
	rt->create(game->window_width, game->window_height, false);
}

void PlayState::onEnter()
{
	cout << "$ Entering play state -- ..." << endl;
	
	player = World::getInstance()->playerAir;
	PlayerController::getInstance()->setPlayer(player);

	// views things
	current_view = 0;

	//set the clear color (the background color)
	glClearColor(0.5, 0.7, 0.8, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusi  ons using the Z buffer

	// Sounds
	if (game->music_enabled)
	{
		SoundManager::getInstance()->playSound("music", true);
		SoundManager::getInstance()->setVolume("music", game->BCK_VOL);

	}

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse

}

void PlayState::render()
{
	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->current_camera->set();
	
	// render to texture

	rt->enable();
	
	//set the clear color (the background color)
	glClearColor(0.5, 0.7, 0.8, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderWorld(game->current_camera);

	rt->disable();

	Camera cam2D;
	cam2D.setOrthographic(-1, 1, -1, 1, -1, 1);
	cam2D.set();

	glDisable(GL_DEPTH_TEST);
	
	Mesh quad;
	quad.createQuad(0, 0, 2, 2);
	
	Shader* shader = Shader::Load("data/shaders/fx.vs", "data/shaders/fx.fs");
	glColor4f(1, 1, 1, 1);

	rt->bind();
	shader->enable();
	shader->setMatrix44("u_mvp", cam2D.viewprojection_matrix);
	shader->setTexture("u_texture", rt);
	shader->setFloat("u_time", game->time);
	quad.render(GL_TRIANGLES, shader);
	shader->disable();
	rt->unbind();
	
	renderGUI();
}

void PlayState::update(double seconds_elapsed)
{

	if(game->keystate[SDL_SCANCODE_7])
		player->speed = 1;

	Vector3 p = player->getPosition();
	std::cout << p.x << ".." << p.y << ".." << p.z << std::endl;

	PlayerController* player_controller = PlayerController::getInstance();

	if (game->current_camera == game->free_camera)
		player_controller->updateCamera(game->current_camera, seconds_elapsed);

	if (pause)
		return;

	// get last position before updating
	player->last_position = player->getPosition();

	// ********************************************************************

	if (game->current_camera == game->fixed_camera)
	{	
		player_controller->update(seconds_elapsed);
	}

	// update all scene
	world->root->update(seconds_elapsed);
	BulletManager::getInstance()->update(seconds_elapsed);
	Explosion::update(seconds_elapsed);

	// interpolate current and previous camera
	Vector3 eye = player->model * viewpos;

	// evitamos que se mueva en la vista de cabina
	if(current_view == FULLVIEW)
		eye = game->current_camera->eye * 0.85 + eye * 0.15;
	
	game->fixed_camera->lookAt(eye, player->model * viewtarget, player->model.rotateVector(Vector3(0, 1, 0)));

	if (controlIA == 1)
		game->free_camera->lookAtPlane((Airplane*)Entity::getEntity("ship"));
	if (controlIA == 2)
		game->free_camera->lookAtPlane((Airplane*)Entity::getEntity("ia_1"));
	if (controlIA == 3)
		game->free_camera->lookAtPlane((Airplane*)Entity::getEntity("ia_2"));
	if (controlIA == 4)
		game->free_camera->lookAtPlane((Airplane*)Entity::getEntity("ia_3"));

	// borrar pendientes
	Entity::destroy_entities();

	// comprobar si es fin del juego
	if (world->isGameOver())
	{
		std::cout << "***** Game has finished! *****" << std::endl;
		system("pause");
		exit(1);
		//SManager->changeCurrentState(EndingState::getInstance(SManager));
	}

}

void PlayState::renderWorld(Camera * camera)
{
	glDisable(GL_DEPTH_TEST);
	world->sky->model.setIdentity();
	world->sky->model.traslate(camera->eye.x, camera->eye.y, camera->eye.z);
	world->sky->render(camera);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Entity * sea = Entity::getEntity("sea");
	sea->model.setIdentity();
	sea->model.traslate(camera->eye.x, -10, camera->eye.z);
	sea->render(camera);

	world->root->render(camera);
	bManager->render();

	Explosion::render(camera);

	//
	if (debug_mesh.vertices.size())
	{
		debug_mesh.render(GL_POINTS);
	}
	//
}

void PlayState::renderGUI() {

	// RENDER GUI ****************************************************************

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	cam2D.set();
	
	Texture::Get(crosshair_tex.c_str())->bind();
	quad.render(GL_TRIANGLES);
	Texture::Get(crosshair_tex.c_str())->unbind();

	// M60 overheat HUD

	std::stringstream ss;

	// Overheat bar
	Mesh heatIndicator;

	float indicatorWidth = !player->shootingtime ? 5 : game->window_height * player->shootingtime * 0.0075;
	float pRcolor = player->shootingtime / 50.f;
	Vector3 v(pRcolor, 1.f - pRcolor, 0.f);

	heatIndicator.createBox(game->window_width*0.7 + game->window_height * 25 * 0.0075, game->window_height*0.95, game->window_height * 50 * 0.0075, 25);
	heatIndicator.render(GL_LINES);

	heatIndicator.createQuad(game->window_width*0.7 + indicatorWidth * 0.5, game->window_height*0.95, indicatorWidth, 25);
	glColor4f(v.x, v.y, v.z, 1.f);
	heatIndicator.render(GL_TRIANGLES);

	ss.str("");
	ss << (int)min(((float)player->shootingtime / 50) * 100, 100.f ) << "%";
	drawText(game->window_width * 0.81, game->window_height * 0.94, ss.str(), Vector3(1.f, 1.f, 1.f), 2.0); // % engine !!!

	if (player->overused)
	{
		drawText(game->window_width * 0.22, game->window_height * 0.935, "--ALERT-- ENGINE OVERHEAT: COOLING SYSTEM", Vector3(1.f, 0.f, 0.f), 2);
	}

	// MISSILES

	ss.str("");
	ss << "Torpedos left: " << player->torpedosLeft;
	drawText(game->window_width*0.75, game->window_height*0.97, ss.str(), Vector3(1, 0, 0), 2.0);

	// vidas enemigas
	int i = 0;
	
	if (EntityCollider::dynamic_colliders.size())
	{
		for (i = 0; i < EntityCollider::dynamic_colliders.size(); i++)
		{
			ss.str("");

			ss << EntityCollider::dynamic_colliders[i]->life << ": " << EntityCollider::dynamic_colliders[i]->name;
			Vector3 c = EntityCollider::dynamic_colliders[i]->name == "player" ? Vector3(0, 1, 0) : Vector3(1, 0, 0);
			drawText(game->window_width*0.1, 50 + i * 25, ss.str(), c, 1.5);
		}
	}

	// set mark enemy airplanes
	Texture * t = Texture::Get("data/textures/mark.tga");
	t->bind();

	Camera* cam3D = Game::instance->current_camera;
	cam2D.set();

	for (int i = 0; i < World::instance->airplanes.size(); i++)
	{
		Entity* current = World::instance->airplanes[i];

		if (current == player)
			continue;

		Vector3 pos3D = current->getPosition();
		Vector3 pos2D = cam3D->project(pos3D, game->window_width, game->window_height);

		if (pos2D.z > 1)
			continue;

		if (pos2D.x < 0)
			pos2D.x = 0;
		if (pos2D.x > game->window_width)
			pos2D.x = game->window_width;
		if (pos2D.y < 0)
			pos2D.y = 0;
		if (pos2D.y > game->window_height)
			pos2D.y = game->window_height;

		float size = cam3D->getProjectScale(pos3D, 50.0);
		if (size < 25.0)
			size = 25.0;
		if (size > 50.0)
			size = 50.0;

		float gradual = (size - 25) / (50 - 25);
		glColor4f(1.0, 1.0, 1.0, 1-  gradual);
		
		Mesh quads;
		quads.createQuad(pos2D.x, game->window_height - pos2D.y, size, size);
		quads.render(GL_TRIANGLES);
	}

	t->unbind();

	// minimap

	Camera camUp;
	camUp.setPerspective(45.f, game->window_width / game->window_height, 0.01, 100000);
	Vector3 center = player->model * Vector3();
	Vector3 eye = center + Vector3(0, 7000, 0);
	Vector3 up = Vector3(0, 0, 1);
	camUp.lookAt(eye, center, up);
	camUp.set();

	int sizex = 195;
	int sizey = 150;

	if (game->keystate[SDL_SCANCODE_LCTRL])
	{
		sizex = 520;
		sizey = 400;
	}

	glScissor(15, 15, sizex, sizey);
	glEnable(GL_SCISSOR_TEST);
	glViewport(15, 15, sizex, sizey);

	glClear(GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < World::instance->map_entities.size(); i++)
	{
		Entity* current = World::instance->map_entities[i];
		current->render(&camUp);
	}

	Mesh objectsInMap;

	Vector3 p1 = center + Vector3(0, 2000, 0);
	Vector3 p2 = center + Vector3(0, 2100, 0);

	//std::cout << p1.y << std::endl; std::cout << p2.y << std::endl;

	objectsInMap.vertices.push_back(p2);
	objectsInMap.vertices.push_back(p1);

	objectsInMap.colors.push_back(Vector4(0.95, 0.85, 0.05, 1.0));
	objectsInMap.colors.push_back(Vector4(0.1, 0.2, 0.96, 0.75));

	for (int i = 0; i < World::instance->airplanes.size(); i++)
	{
		Entity* current = World::instance->airplanes[i];
		Vector3 current_center = current->model * Vector3() + Vector3(0, 1500, 0);
		objectsInMap.vertices.push_back(current_center);
		objectsInMap.colors.push_back(Vector4(0.9, 0.0, 0.0, 0.75));
	}
	
	Shader* shader = Shader::Load("data/shaders/map.vs", "data/shaders/map.fs");

	Matrix44 m;

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", camUp.viewprojection_matrix);
	shader->setVector3("u_camera_pos", camUp.eye);

	objectsInMap.render(GL_POINTS, shader);
	
	shader->disable();

	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// FINISHED RENDER INTERFACE ****************************************************************
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_BLEND);
	glViewport(0, 0, game->window_width, game->window_height);
}

void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_0:
		game->start = true;
		player->engineOnOff();
		SoundManager::getInstance()->playSound("plane", true);
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
	case SDLK_6:
		game->current_camera = game->free_camera;
		controlIA++;
		break;
	case SDLK_p:
		pause = !pause;
		break;
	}
}

void PlayState::onKeyUp(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_SPACE:
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
		setZoom();
	}
}

void PlayState::setZoom()
{
	// si es el model 2, ir� a la posici�n 2*2 = 4(full plane). Si current view = 0, 4+4 = 0(full plane)
	// si cv = 1, 4+1 = 5 que es de la cabina
	if (inZoom)
	{
		viewpos.z += vTranslations[world->worldInfo.playerModel * 2 + current_view].qnt;
		game->current_camera->near_plane = 2.5f;
		game->current_camera->far_plane = 50000.f;
	}
	else
		setView();
}

void PlayState::onLeave(int fut_state)
{
	SoundManager::getInstance()->stopSound("music");
}

void PlayState::setView() {

	int plane_model = player->planeModel;

	switch (current_view)
	{
	case FULLVIEW:

		game->current_camera->near_plane = 7.5f;
		game->current_camera->far_plane = 50000.f;

		if (plane_model == SPITFIRE)
		{
			player->set("spitfire.ASE", "data/textures/spitfire.tga", "plane");
		}

		viewpos = Vector3(0.f, 6.f, -6.5f);
		viewtarget = Vector3(0.f, 5.f, 10.f);
		break;

	case CABINEVIEW:

		game->current_camera->near_plane = 0.1f;
		game->current_camera->far_plane = 50000.f;
		
		if (plane_model == SPITFIRE)
		{
			viewpos = Vector3(0.f, 0.7f, -1.5f);
			viewtarget = Vector3(0.f, 0.0f, 100.f);
			player->set("spitfire_cabina.ASE", "data/textures/spitfire_cabina_alpha.tga", "simple");

		}
		else if (plane_model == P38)
		{
			viewpos = Vector3(-0.03f, 0.75f, 0.752f);
			viewtarget = Vector3(0.f, 0.5f, 10.f);
		} 
		else if (plane_model == WILDCAT)
		{
			viewpos = Vector3(0.f, 1.f, -0.5f);
			viewtarget = Vector3(0.f, 1.f, 10.f);
		}
		else if (plane_model == BOMBER)
		{
			viewpos = Vector3(0.5f, 0.f, 2.5f);
			viewtarget = Vector3(0.f, 1.f, 10.f);
		}

		break;
	}
}