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
#include "../effects.h"
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

RenderToTexture * rt = NULL;

PlayState::PlayState(StateManager* SManager) : State(SManager)
{
	inZoom = false;
}

PlayState::~PlayState() {}

void PlayState::init()
{
	bManager = BulletManager::getInstance();
	game = Game::getInstance();
	world = World::getInstance();
	world->create();

	// posicion y direccion de la vista seleccionada
	viewpos = Vector3(0.f, 7.f, -11.f);
	viewtarget = Vector3(0.f, 0.f, 100.f);

	//create our camera
	game->fixed_camera = new Camera();
	game->fixed_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 0.01f, 50000.f);

	game->free_camera = new Camera(); //our global camera
	game->free_camera->lookAt(Vector3(135, 50, -410), Vector3(0,0, 0), Vector3(0, 1, 0));
	game->free_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 1.0, 25000.f);

	game->shooter_camera = new Camera();
	game->shooter_camera->setPerspective(70.f, game->window_width / (float)game->window_height, 7.5f, 50000.f);

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

	// set collision models

	for (int i = 0; i < EntityCollider::static_colliders.size(); i++)
	{
		EntityCollider * current_collider = EntityCollider::static_colliders[i];
		Mesh::Get(current_collider->mesh.c_str())->setCollisionModel();
	}

	crosshair_tex = "data/textures/crosshair.tga";

	rt = new RenderToTexture();
}

void PlayState::onEnter()
{
	cout << "$ Entering play state -- ..." << endl;
	
	rt->create(game->window_width, game->window_height, false);
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
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
	SoundManager::getInstance()->setVolume("music", game->BCK_VOL);
	SoundManager::getInstance()->playSound("kick", false);
	SoundManager::getInstance()->setVolume("kick", 0.5);

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse

	refs_timer = 120;
	playTime = 0;
}

void PlayState::render()
{
	int w = game->window_width;
	int h = game->window_height;

	// render to texture
	rt->enable();

	//set the clear color (the background color)
	glClearColor(0.5, 0.7, 0.8, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//
	if (current_view == SHOOTERVIEW)
	{
		game->shooter_camera->set();
		renderWorld(game->shooter_camera);
	
		glScissor(w * 0.7, h * 0.4, 300, 200);
		glEnable(GL_SCISSOR_TEST);
		glViewport(w * 0.7, h * 0.4, 300, 200);

		glClear(GL_DEPTH_BUFFER_BIT);

		game->current_camera->set();
		renderWorld(game->current_camera);

		glDisable(GL_SCISSOR_TEST);
		glViewport(0, 0, w, h);
	}
	else
	{
		game->current_camera->set();
		renderWorld(game->current_camera);
	}
	//

	rt->disable();

	Camera cam2D;
	cam2D.setOrthographic(-1, 1, -1, 1, -1, 1);
	cam2D.set();

	glDisable(GL_DEPTH_TEST);
	
	Mesh quad;
	quad.createQuad(0, 0, 2, 2);
	
	Shader* shader = Shader::Load("data/shaders/fx.vs", "data/shaders/fx.fs");
	glColor4f(1, 1, 1, 1);

	// render to texture
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
	//Vector3 p = player->getPosition();
	//std::cout << p.x << ".." << p.y << ".." << p.z << std::endl;

	PlayerController* player_controller = PlayerController::getInstance();

	if (game->current_camera == game->free_camera)
		player_controller->updateCamera(game->current_camera, seconds_elapsed);

	if (pause)
		return;

	if (world->atomic_enabled)
	{
		world->time_to_explode -= seconds_elapsed;

		if (world->time_to_explode < 0)
		{
			world->atomic_enabled = false;
			world->atomic();
		}
	}

	playTime += seconds_elapsed;
	refs_timer -= seconds_elapsed;

	if (refs_timer < 0)
	{
		World::instance->createReinforcements();
		refs_timer = 100;
	}

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

	// update effects
	Explosion::update(seconds_elapsed);
	Flash::update(seconds_elapsed);
	Smoke::update(seconds_elapsed);

	// interpolate current and previous camera
	Vector3 eye = player->model * viewpos;
	Vector3 s_eye = player->model * Vector3(0.f, 0.f, 0.f);

	// evitamos que se mueva en la vista de cabina
	if(current_view == FULLVIEW && game->start)
		eye = game->current_camera->eye * 0.85 + eye * 0.15;
	
	game->fixed_camera->lookAt(eye, player->model * viewtarget, player->model.rotateVector(Vector3(0, 1, 0)));
	game->shooter_camera->lookAt(s_eye, player->model * Vector3(0.f, 5.f, -100.f), player->model.rotateVector(Vector3(0, 1, 0)));

	// borrar pendientes
	Entity::destroy_entities();

	// comprobar si es fin del juego
	if (world->isGameOver())
	{
		std::cout << "***** Game has finished! *****" << std::endl;
		game->end = true;
		SManager->changeCurrentState(EndingState::getInstance(SManager));
	}

}

void PlayState::renderWorld(Camera * camera)
{
	glDisable(GL_DEPTH_TEST);
	Entity * sky = Entity::getEntity("sky");
	sky->model.setIdentity();
	sky->model.traslate(camera->eye.x, camera->eye.y, camera->eye.z);
	sky->render(camera);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	Entity * sea = Entity::getEntity("sea");
	sea->model.setIdentity();
	sea->model.traslate(camera->eye.x, -10, camera->eye.z);
	sea->render(camera);

	world->root->render(camera);
	bManager->render();

	// render effects
	Explosion::render(camera);
	Flash::render(camera);
	Smoke::render(camera);

	//
	if (debug_mesh.vertices.size())
	{
		debug_mesh.render(GL_POINTS);
	}
	//
}

void PlayState::renderGUI()
{
	int w = game->window_width;
	int h = game->window_height;
	std::stringstream ss;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	cam2D.set();
	
	if (current_view == SHOOTERVIEW)
	{
		// crosshair
		Shader* shader = Shader::Load("data/shaders/fx.vs", "data/shaders/fx.fs");
		quad.createQuad(w * 0.5, h * 0.85, w * 0.5, h * 0.3, true);
		Texture::Get("data/textures/minigun.tga")->bind();
		shader->enable();
		shader->setMatrix44("u_mvp", cam2D.viewprojection_matrix);
		shader->setTexture("u_texture", Texture::Get("data/textures/minigun.tga"));
		shader->setFloat("u_time", game->time);
		quad.render(GL_TRIANGLES, shader);
		shader->disable();
		Texture::Get("data/textures/minigun.tga")->unbind();
	}
	else
	{
		// crosshair
		quad.createQuad(w * 0.5, h * 0.475, 50, 50, true);
		Texture::Get(crosshair_tex.c_str())->bind();
		quad.render(GL_TRIANGLES);
		Texture::Get(crosshair_tex.c_str())->unbind();
	}

	glColor4f(1, 1, 1, 1.0);
	glLineWidth(1.0);

	Mesh energyBar;

	energyBar.createBox(w*0.7 + h * 25 * 0.0075, h*0.9, h * 50 * 0.0075, 25);
	energyBar.render(GL_LINES);

	// fondo barra
	float full = h * 50 * 0.0075;

	energyBar.createQuad(w*0.7 + full * 0.5, h*0.9, full, 25);
	glColor4f(0.f, 0.f, 0.f, 1.f);
	energyBar.render(GL_TRIANGLES);

	float f = (player->life / (float)player->max_life);
	float energyWidth = f * h * 50 * 0.0075;
	Vector3 ve(0.8, 0.8, 0.1);

	energyBar.createQuad(w*0.7 + energyWidth * 0.5, h*0.9, energyWidth, 25);
	glColor4f(ve.x, ve.y, ve.z, 1.f);
	energyBar.render(GL_TRIANGLES);

	// Overheat bar
	Mesh heatIndicator;
	glColor4f(1.0, 1.0, 1.0, 1.f);

	float indicatorWidth = !player->shootingtime ? 5 : h * player->shootingtime * 0.0075;
	float pRcolor = player->shootingtime / 50.f;
	Vector3 v(pRcolor, 1.f - pRcolor, 0.f);

	heatIndicator.createBox(w*0.7 + h * 25 * 0.0075, h*0.93, h * 50 * 0.0075, 10);
	heatIndicator.render(GL_LINES);

	heatIndicator.createQuad(w*0.7 + indicatorWidth * 0.5, h*0.93, indicatorWidth, 10);
	glColor4f(v.x, v.y, v.z, 1.f);
	heatIndicator.render(GL_TRIANGLES);

	// engine
	ss.str("");
	ss << (int)min(((float)player->shootingtime / 50) * 100, 100.f ) << "%";
	drawText(w * 0.81, h * 0.95, ss.str(), Vector3(1.f, 1.f, 1.f), 2.0); // % engine !!!

	// energy
	ss.str("");
	ss << player->life;
	drawText(w * 0.81, h * 0.89, ss.str(), Vector3(1.f, 1.f, 1.f), 2.0);

	if (player->overused)
	{
		drawText(w * 0.25, h * 0.935, "--ALERT-- ENGINE OVERHEAT: COOLING SYSTEM", Vector3(1.f, 0.f, 0.f), 2.f);
	}

	// TIMER REFUERZOS

	int mins = refs_timer / 60.f;
	int seconds = refs_timer - 60 * mins;
	ss.str("");
	ss << mins << ":";
	if(seconds < 10)
		ss << "0" << seconds;
	else
		ss << seconds;
	drawText(w * 0.05, h * 0.65, "Reinforcements coming in", Vector3(1.f, 0.f, 1.f), 2.f);
	drawText(w * 0.125, h * 0.7, ss.str(), Vector3(1.f, 0.f, 1.f), 3.5f);

	// TIMER BOMBA

	if (World::instance->atomic_enabled)
	{
		mins = World::instance->time_to_explode / 60.f;
		seconds = World::instance->time_to_explode - 60 * mins;
		ss.str("");
		ss << mins << ":";
		if (seconds < 10)
			ss << "0" << seconds;
		else
			ss << seconds;
		drawText(w * 0.5, h * 0.5, ss.str(), Vector3(1.f, 0.f, 0.f), 5.f);
	}

	//

	glColor4f(1, 1, 1, 1.0);

	// cantidad de players

	cam2D.set();

	Mesh logos;
	Texture * et = Texture::Get("data/textures/allies-logo.tga");
	logos.createQuad(w * 0.05, h * 0.1, h * 0.1, h * 0.1, true);

	et->bind();
	logos.render(GL_TRIANGLES);
	et->unbind();

	et = Texture::Get("data/textures/enemies-logo.tga");
	logos.createQuad(w * 0.05, h * 0.2, h * 0.1, h * 0.1, true);

	et->bind();
	logos.render(GL_TRIANGLES);
	et->unbind();

	// missiles

	glColor4f(1, 1, 1, 1.0);
	Texture * etor = Texture::Get("data/textures/torpedo-icon.tga");

	for (int i = 0; i < player->torpedosLeft; i++)
	{
		Mesh torpedo;
		torpedo.createQuad(w * 0.91, h * 0.83 + i * 20, h * 0.075, h * 0.075);
		etor->bind();
		torpedo.render(GL_TRIANGLES);
		etor->unbind();
	}
	// players

	ss.str("");
	ss << "Enemies: " << (1 + World::instance->airplanes.size());
	drawText(w * 0.1, h * 0.2, ss.str(), Vector3(1, 1, 1), 2);
	drawText(w * 0.1, h * 0.1, "Allies: 1", Vector3(1, 1, 1), 2);

	if (World::instance->ships.size())
	{
		for (int i = 0; i < World::instance->ships.size(); i++)
		{
			Ship* c = (Ship*)World::instance->ships[i];
			ss.str("");
			Vector3 color;
			if (c->uid == Airplane::PLAYER_SHIP)
			{
				ss << "DEFEND: " << c->life;
				color = Vector3(0, 0, 1);
			}
			else
			{
				ss << "ATTACK: " << c->life;
				color = Vector3(1, 0, 0);
			}
			drawText(w * 0.04, h * 0.3 + i * h * 0.05, ss.str(), color, 3.0);
		}
	}

	// in game debug info
	if (game->inGame_DEBUG)
	{
		if (World::instance->airplanes.size())
		{
			for (int i = 0; i < World::instance->airplanes.size(); i++)
			{
				Airplane* c = (Airplane*)World::instance->airplanes[i];
				ss.str("");
				ss << c->name << " : " << c->life << " : " << c->controller->state;
				drawText(w * 0.2, 50 + i * 25, ss.str(), Vector3(1, 0, 0), 1.5);
			}
		}

		ss.str("");
		ss << "Alt(Y): " << (int)player->getPosition().y;
		drawText(w * 0.4, 50, ss.str(), Vector3(1, 0, 1), 1.5);
		ss.str("");
		ss << "Coord(X, Z): (" << (int)player->getPosition().x << "),  (" << (int)player->getPosition().z << ")";
		drawText(w * 0.4, 75, ss.str(), Vector3(1, 0, 1), 1.5);

		ss.str("");
		ss << "Damage: " << player->damageM60 << " || Max energy: " << player->max_life;
		drawText(w * 0.64, 50, ss.str(), Vector3(0.9, 0.9, 0.1), 1.5);
		ss.str("");
		ss << "Torpedos: " << 2 << " || Torpedo damage: " << 450;
		drawText(w * 0.64, 75, ss.str(), Vector3(0.9, 0.9, 0.1), 1.5);
	}
	

	// set mark enemy airplanes
	Texture * t = Texture::Get("data/textures/mark.tga");

	Camera* cam3D = current_view == SHOOTERVIEW ? game->shooter_camera : game->current_camera;
	cam2D.set();

	for (int i = 0; i < World::instance->airplanes.size(); i++)
	{
		Airplane* current = (Airplane*)World::instance->airplanes[i];

		if (current == player)
			continue;

		Vector3 pos3D = current->getPosition();
		Vector3 pos2D = cam3D->project(pos3D, w, h);

		if (pos2D.z > 1)
			continue;

		if (pos2D.x < 0)
			pos2D.x = 0;
		if (pos2D.x > w)
			pos2D.x = w;
		if (pos2D.y < 0)
			pos2D.y = 0;
		if (pos2D.y > h)
			pos2D.y = h;

		float size = cam3D->getProjectScale(pos3D, 50.0);
		if (size < 25.0)
			size = 25.0;
		if (size > 50.0)
			size = 50.0;

		float gradual = (size - 25) / (50 - 25);
		glColor4f(1.0, 1.0, 1.0, 1 - gradual);
		
		Mesh quads;
		quads.createQuad(pos2D.x, h - pos2D.y, size, size);
		t->bind();
		quads.render(GL_TRIANGLES);
		t->unbind();

		// lifes
		quads.createQuad(pos2D.x + 1, h - pos2D.y - 20, size * current->life * 0.003, size / 10);
		float f = current->life / (float)current->max_life;
		glColor4f(1 - f, f, 0.0, 0.4 + gradual);
		quads.render(GL_TRIANGLES);
	}

	

	// minimap

	Camera camUp;
	camUp.setPerspective(45.f, w / (float)h, 0.01, 100000);
	Vector3 center = player->model * Vector3();
	Vector3 eye = center + Vector3(0, 3500, 0);
	Vector3 up = Vector3(0, 0, 1);
	camUp.lookAt(eye, center, up);
	camUp.set();

	int sizex = w * 0.2;
	int sizey = h * 0.23;

	glScissor(w * 0.04, h - h * 0.99, sizex, sizey);
	glEnable(GL_SCISSOR_TEST);
	glViewport(w * 0.04, h - h * 0.99, sizex, sizey);

	glClear(GL_DEPTH_BUFFER_BIT);

	for (int i = 0; i < World::instance->map_entities.size(); i++)
	{
		Entity* current = World::instance->map_entities[i];
		current->render(&camUp);
	}

	Mesh objectsInMap;
	Mesh visionField;

	// player
	objectsInMap.vertices.push_back(center);
	objectsInMap.colors.push_back(Vector4(0.9, 0.75, 0.0, 0.65));

	Vector3 camera = Game::instance->current_camera->center;

	visionField.vertices.push_back(camera);
	Vector3 p1 = camera + player->model.rotateVector(Vector3(200, 0, 300));
	Vector3 p2 = camera + player->model.rotateVector(Vector3(-200, 0, 300));
	visionField.vertices.push_back(p1);
	visionField.vertices.push_back(p2);
	visionField.colors.push_back(Vector4(0.0, 0.9, 0.0, 0.0));
	visionField.colors.push_back(Vector4(0.0, 0.9, 0.0, 1.0));
	visionField.colors.push_back(Vector4(0.0, 0.9, 0.0, 1.0));

	for (int i = 0; i < World::instance->airplanes.size(); i++)
	{
		Entity* current = World::instance->airplanes[i];
		Vector3 current_center = current->model * Vector3();
		objectsInMap.vertices.push_back(current_center);
		objectsInMap.colors.push_back(Vector4(0.9, 0.0, 0.0, 0.65));
	}

	for (int i = 0; i < World::instance->powerups.size(); i++)
	{
		Entity* current = World::instance->powerups[i];
		Vector3 current_center = current->model * Vector3();
		objectsInMap.vertices.push_back(current_center);
		objectsInMap.colors.push_back(Vector4(0.9, 0.0, 0.9, 0.5));
	}

	for (int i = 0; i < World::instance->ships.size(); i++)
	{
		Entity* current = World::instance->ships[i];
		Vector3 current_center = current->model * Vector3();
		objectsInMap.vertices.push_back(current_center);
		objectsInMap.colors.push_back(Vector4(0, 0.0, 0.9, 0.65));
	}
	
	Shader* shader = Shader::Load("data/shaders/map.vs", "data/shaders/map.fs");

	Matrix44 m;

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	shader->enable();
	shader->setMatrix44("u_model", m);
	shader->setMatrix44("u_mvp", camUp.viewprojection_matrix);
	shader->setVector3("u_camera_pos", camUp.eye);
	glPointSize(8.0);
	objectsInMap.render(GL_POINTS, shader);
	shader->disable();
	glLineWidth(8.0);
	visionField.render(GL_TRIANGLES);
	glLineWidth(1.0);
	glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, w, h);
	
	// minimap border

	cam2D.set();

	Mesh border;
	Texture * tc = Texture::Get("data/textures/compass.tga");
	border.createQuad(w * 0.14, h * 0.875, sizex, sizex, true);

	glColor4f(1, 1, 1, 1.0);
	glLineWidth(1.0);
	tc->bind();
	border.render(GL_TRIANGLES);
	tc->unbind();

	// compass

	glColor4f(1, 1, 1, 1.0);
	Texture * bru = Texture::Get("data/textures/brujula.tga");

	cam2D.set();

	Mesh compass;
	int tam = 200;
	compass.createQuad(w * 0.9, h * 0.15, tam, tam, true);
	bru->bind();
	compass.render(GL_TRIANGLES);
	bru->unbind();

	// compass indicators

	camUp.setPerspective(45.f, w / (float)h, 0.01, 100000);
	center = player->model * Vector3();
	eye = center + Vector3(0, 2000, 0);
	up = Vector3(0, 0, 1);
	camUp.lookAt(eye, center, up);
	camUp.set();

	glScissor(w * 0.9 - tam * 0.5, h - h * 0.15 - tam * 0.5, tam, tam);
	glEnable(GL_SCISSOR_TEST);
	glViewport(w * 0.9 - tam * 0.5, h - h * 0.15 - tam * 0.5, tam, tam);

	glClear(GL_DEPTH_BUFFER_BIT);

	Mesh agu;

	Vector3 pos1 = player->getPosition();
	Vector3 pos2 = player->model * Vector3(0, 0, 400);
	Vector3 pos3 = player->model * Vector3(0, 0, -400);

	agu.vertices.push_back(pos1);
	agu.vertices.push_back(pos2);
	agu.vertices.push_back(pos1);
	agu.vertices.push_back(pos3);

	agu.colors.push_back(Vector4(1, 0.0, 0.0, 0.95));
	agu.colors.push_back(Vector4(1, 0.0, 0.0, 0.15));
	agu.colors.push_back(Vector4(0, 0.2, 0.9, 0.95));
	agu.colors.push_back(Vector4(0, 0.2, 0.9, 0.15));

	glLineWidth(5);
	agu.render(GL_LINES);
	glLineWidth(1.0);
	glDisable(GL_SCISSOR_TEST);
	glViewport(0, 0, w, h);

	
	// FINISHED RENDER INTERFACE ****************************************************************
	
	glDisable(GL_BLEND);
}

void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	std::string cheat;

	switch (event.keysym.sym)
	{
	case SDLK_0:
		game->start = true;
		game->current_camera->near_plane = 7.5f;
		SoundManager::getInstance()->playSound("plane", true);
		break;
	case SDLK_1:
		game->free_camera->lookAt(player->getPosition() + Vector3(0, 0, -100), Vector3(0, 0, 1000), Vector3(0, 1, 0));
		game->current_camera = game->current_camera == game->fixed_camera ? game->free_camera : game->fixed_camera;
		break;
	case SDLK_2:
		PlayerController::getInstance()->current_controller = CONTROLLER_MODE_KEYBOARD;
		break;
	case SDLK_3:
		Shader::ReloadAll();
		break;
	case SDLK_h:
		if (game->fullscreen)
			return;
		std::cin >> cheat;
		if (cheat == "god")
			player->damageM60 = 1000.0;
		else if (cheat == "machine")
			player->cadence = 150;
		else if (cheat == "tank")
			player->setLife(1000);
		else if (cheat == "bolt")
			player->speed = 200.0;
		break;
		// full plane view or cabine view
	case SDLK_v: 
		current_view++;
		if (current_view > SHOOTERVIEW)
			current_view = FULLVIEW;
		setView();
		break;
		// pause playstate
	case SDLK_p:
		pause = !pause;
		break;
		// debug info
	case SDLK_i:
		game->inGame_DEBUG = !game->inGame_DEBUG;
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

void PlayState::onMouseButton(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_RIGHT) //right mouse
	{
		inZoom = !inZoom;
		setZoom();
	}
}

void PlayState::setZoom()
{
	// si es el model 2, irá a la posición 2*2 = 4(full plane). Si current view = 0, 4+4 = 0(full plane)
	// si cv = 1, 4+1 = 5 que es de la cabina
	if (inZoom)
	{
		viewpos.z += vTranslations[player->planeModel * 2 + current_view].qnt;
		if (current_view == CABINEVIEW)
		{
			game->current_camera->near_plane = 0.01f;
			game->current_camera->far_plane = 40000;
		}

	}
	else
		setView();
}

void PlayState::onLeave(int fut_state)
{
	if (fut_state == 0)
	{
		SoundManager::getInstance()->stopSound("plane");
		return;
	}
	
	SoundManager::getInstance()->stopSound("plane");
	SoundManager::getInstance()->stopSound("music");
	game->bkg_music_playing = !game->bkg_music_playing;
}

void PlayState::setView()
{
	int plane_model = player->planeModel;

	switch (current_view)
	{
	case FULLVIEW:

		game->current_camera->near_plane = 7.5f;
		game->current_camera->far_plane = 50000.f;

		if (plane_model == SPITFIRE)
		{
			player->set("spitfire.ASE", "data/textures/spitfire.tga", "spitfire");
		}

		viewpos = Vector3(0.f, 7.f, -11.f);
		viewtarget = Vector3(0.f, 0.f, 100.f);
		break;

	case SHOOTERVIEW:
		// same representation as fullview
		game->current_camera->near_plane = 7.5f;
		game->current_camera->far_plane = 50000.f;

		if (plane_model == SPITFIRE)
		{
			player->set("spitfire.ASE", "data/textures/spitfire.tga", "spitfire");
		}

		viewpos = Vector3(0.f, 7.f, -11.f);
		viewtarget = Vector3(0.f, 0.f, 100.f);
		break;

	case CABINEVIEW:

		game->current_camera->near_plane = 0.1f;
		game->current_camera->far_plane = 20000.f;
		
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

		break;
	}
}