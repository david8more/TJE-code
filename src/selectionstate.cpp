#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "texturemanager.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "selectionstate.h"
#include "playstate.h"
#include "entity.h"
#include "world.h"

#define DEBUG 0

EntityMesh* eMesh = NULL; // plane mesh
EntityMesh* bMesh = NULL; // background mesh
EntityMesh* gMesh = NULL; // ground mesh
Camera* cam3D = NULL;

PreplayState::PreplayState(StateManager* SManager) : State(SManager) {}
PreplayState::~PreplayState() {}

PreplayState* PreplayState::getInstance(StateManager* SManager)
{
	static PreplayState Instance(SManager);
	return &Instance;
}

void PreplayState::onKeyPressed(SDL_KeyboardEvent event)
{
	World* world = World::getInstance();

	if (DEBUG) return;

	switch (event.keysym.sym)
	{
	case SDLK_RETURN:
		//set the clear color (the background color) & clear the window and the depth buffer
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// tell world the plane selected
		world->worldInfo.playerModel = playerModel;
		world->addEnemies();
		world->addPlayer();
		SManager->changeCurrentState(PlayState::getInstance(SManager));
		break;
	case SDLK_UP:
	case SDLK_w:
		s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
		playerModel--;
		if (playerModel == -1) playerModel = 3;
		break;
	case SDLK_DOWN:
	case SDLK_s:
		s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
		playerModel++;
		if (playerModel == 4) playerModel = 0;
		break;
	}
}

void PreplayState::init() {

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	//create our camera
	cam3D = new Camera();
	cam3D->lookAt(Vector3(0.f, 5.f, 15.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	cam3D->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	// create semi-WORLD

	// plane
	eMesh = new EntityMesh();
	eMesh->set("spitfire.ASE", "data/textures/spitfire.tga", "new");
	eMesh->model.rotateLocal(0.5f, Vector3(0, -1, 0));

	// sky
	bMesh = new EntityMesh();
	bMesh->set("cielo.ASE", "data/textures/cielo.tga", "color");
	bMesh->model.setTranslation(0.f, -500.f, 0.f);
	bMesh->model.scale(30.f, 30.f, 30.f);

	// ground
	gMesh = new EntityMesh();
	gMesh->set("terrain.ASE", "data/textures/terrain.tga", "color");
	//gMesh->model = gMesh->model * bMesh->model;
	gMesh->model.setTranslation(0.f, -500.f, 0.f);
	bMesh->model.scale(0.25f, 0.25f, 0.25f);

	// rendering properties

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.95, game->window_width, game->window_height * 0.1, true);

	texture = TextureManager::getInstance()->getTexture("data/textures/indications.tga");

	// SET PLANE INFO STRINGS

	selectionHelp.resize(4);

	for (int i = 0; i < 4; ++i) {
		selectionHelp[i].resize(6);
	}

	selectionHelp[0][0] = "SPITFIRE";
	selectionHelp[0][1] = "DAMAGE ++";
	selectionHelp[0][2] = "LIFE ++";
	selectionHelp[0][3] = "CADENCE ++";
	selectionHelp[0][4] = "MISSILES +++";
	selectionHelp[0][5] = "SPEED +";

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse
}

void PreplayState::onEnter()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cout << "$ Entering loading state" << endl;

	playerModel = 0;
	lastRendered = 0;
}

void PreplayState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	cam3D->set();

	switch (playerModel) {
	case SPITFIRE:
		if (lastRendered == playerModel) break;
		eMesh->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		break;
	case P38:
		if (lastRendered == playerModel) break;
		eMesh->set("p38.ASE", "data/textures/p38.tga", "simple");
		break;
	case WILDCAT:
		if (lastRendered == playerModel) break;
		eMesh->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		break;
	case BOMBER:
		if (lastRendered == playerModel) break;
		eMesh->set("bomber_axis.ASE", "data/textures/bomber_axis.tga", "simple");
		break;
	default:
		break;
	}
	
	eMesh->render(cam3D);
	lastRendered = playerModel;
	bMesh->render(cam3D);
	gMesh->render(cam3D);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();

	// PLANE INFO

	for (int i = 0; i < 6; i++) {
		drawText(game->window_width*0.75, game->window_height*0.1*(float)(i+1)/2, selectionHelp[0][i], Vector3(1.f, 1.f, 1.f), 2.0);
	}

	// ************************************************

	drawText(game->window_width*0.1, game->window_height*0.935, "W/S - Change", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(game->window_width*0.4, game->window_height*0.935, "A/D - Move", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(game->window_width*0.7, game->window_height*0.935, "ENTER - Select", Vector3(1.f, 1.f, 1.f), 2.0);
	
}

void PreplayState::update(double time_elapsed) {

	float speed = time_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant

	if (DEBUG) {
		//mouse input to rotate the cam
		if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
		{
			cam3D->rotate(game->mouse_delta.x * 0.005f, Vector3(0.0f, -0.5f, 0.0f));
			cam3D->rotate(game->mouse_delta.y * 0.005f, cam3D->getLocalVector(Vector3(-0.5f, 0.0f, 0.0f)));
		}

		//async input to move the camera around
		if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
		if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) cam3D->move(Vector3(0.0f, 0.0f, 0.1f) * speed);
		if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) cam3D->move(Vector3(0.0f, 0.0f, -0.1f) * speed);
		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) cam3D->move(Vector3(0.1f, 0.0f, 0.0f) * speed);
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) cam3D->move(Vector3(-0.1f, 0.0f, 0.0f) * speed);
	}
	else {

		//mouse input to rotate the cam
		if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
		{
			eMesh->model.rotateLocal(game->mouse_delta.x * 0.005f, Vector3(0, 1, 0));
			eMesh->model.rotateLocal(game->mouse_delta.y * 0.005f, Vector3(1, 0, 0));
		}

		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) eMesh->model.rotateLocal(0.05f, Vector3(0, 1, 0));
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) eMesh->model.rotateLocal(0.05f, Vector3(0, -1, 0));
		if (game->keystate[SDL_SCANCODE_B]) cam3D->move(Vector3(0.f, 0.f, 1.f));
		if (game->keystate[SDL_SCANCODE_V]) cam3D->move(Vector3(0.f, 0.f, -1.f));
	}

	//to navigate with the mouse fixed in the middle
	if (game->mouse_locked)
	{
		int center_x = (int)floor(game->window_width*0.5f);
		int center_y = (int)floor(game->window_height*0.5f);

		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = (float)center_x;
		game->mouse_position.y = (float)center_y;
	}

}