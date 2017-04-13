#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "loadingstate.h"
#include "playstate.h"
#include "entity.h"
#include "world.h"
#include <cmath>
#include <ctime>

EntityMesh* eMesh = NULL; // plane mesh
EntityMesh* bMesh = NULL; // background mesh
EntityMesh* gMesh = NULL; // ground mesh
Camera* cam3D = NULL;

float anglerot = 0;
int playerModel = 0;
int lastRendered = 0;

LoadingState::LoadingState(StateManager* SManager) : State(SManager) {}
LoadingState::~LoadingState() {}

LoadingState* LoadingState::getInstance(StateManager* SManager)
{
	static LoadingState Instance(SManager);
	return &Instance;
}

void LoadingState::onKeyPressed(SDL_KeyboardEvent event)
{
	World* world = World::getInstance();

	switch (event.keysym.sym)
	{
	case SDLK_RETURN:
		
		world->worldInfo.playerModel = playerModel;
		PlayState::getInstance(SManager)->init();
		SManager->changeCurrentState(PlayState::getInstance(SManager));
		break;
	case SDLK_UP:
	case SDLK_w:
		playerModel++;
		if (playerModel == 3) playerModel = 0;
		break;
	case SDLK_DOWN:
	case SDLK_s:
		playerModel--;
		if (playerModel == -1) playerModel = 2;
		break;
	}
}

void LoadingState::init() {

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	//create our camera
	cam3D = new Camera();
	cam3D->lookAt(Vector3(0.f, 5.f, 10.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0
	cam3D->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	eMesh = new EntityMesh();
	bMesh = new EntityMesh();
	gMesh = new EntityMesh();
	eMesh->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
	eMesh->model.rotateLocal(0.5f, Vector3(0, -1, 0));
	bMesh->set("cielo.ASE", "data/textures/cielo.tga", "color");
	bMesh->model.setTranslation(0.f, -500.f, 0.f);
	gMesh->set("terrain.ASE", "data/textures/terrain.tga", "color");
	gMesh->model = gMesh->model * bMesh->model;

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width * 0.1, game->window_height*0.075, true);

	texture = new Texture();
	if (!texture->load("data/textures/select.tga")) {
		cout << "Error in loading state: texture has not been loaded" << endl;
		exit(1);
	}

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse
}

void LoadingState::onEnter()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cout << "$ Entering loading state" << endl;
}

void LoadingState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	cam3D->set();

	glEnable(GL_BLEND);

	//Draw out world
	drawGrid(500); //background grid

	switch (playerModel) {
	case 0:
		if (lastRendered == playerModel) break;
		eMesh->set("spitfire.ASE", "data/textures/spitfire.tga", "simple");
		break;
	case 1:
		if (lastRendered == playerModel) break;
		eMesh->set("p38.ASE", "data/textures/p38.tga", "simple");
		break;
	case 2:
		if (lastRendered == playerModel) break;
		eMesh->set("wildcat.ASE", "data/textures/wildcat.tga", "simple");
		break;
	default:
		break;
	}
	eMesh->render(cam3D);
	lastRendered = playerModel;
	bMesh->render(cam3D);
	gMesh->render(cam3D);

	/*glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();*/

	glDisable(GL_BLEND);
}

void LoadingState::update(double time_elapsed) {

	float speed = time_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

										 //mouse input to rotate the cam
	if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
	{
		eMesh->model.rotateLocal(game->mouse_delta.x * 0.005f, Vector3(0, 1, 0));
		eMesh->model.rotateLocal(game->mouse_delta.y * 0.005f, Vector3(1, 0, 0));
	}

	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) eMesh->model.rotateLocal(0.05f, Vector3(0, 1, 0));
	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) eMesh->model.rotateLocal(0.05f, Vector3(0, -1, 0));

	//to navigate with the mouse fixed in the middle
	if (game->mouse_locked)
	{
		int center_x = (int)floor(game->window_width*0.5f);
		int center_y = (int)floor(game->window_height*0.5f);
		//center_x = center_y = 50;
		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen
																 //SDL_WarpMouseGlobal(center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = (float)center_x;
		game->mouse_position.y = (float)center_y;
	}


	anglerot += (float)time_elapsed;
}