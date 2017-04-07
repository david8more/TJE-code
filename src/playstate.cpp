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
World * world = NULL;

PlayState::PlayState(StateManager* SManager) : State(SManager) {}
PlayState::~PlayState() {}

PlayState* PlayState::getInstance(StateManager* SManager)
{
	static PlayState Instance(SManager);
	return &Instance;
}

void PlayState::onKeyPressed(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{

	}
}

void PlayState::init() {

}

void PlayState::onEnter()
{
	cout << "$ Entering play state -- ..." << endl;
	Game* game = Game::getInstance();

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	//create our camera
	game->camera = new Camera();
	game->camera->lookAt(Vector3(0, 1200, 25), Vector3(50, 50, 50), Vector3(0, 1, 0)); //position the camera and point to 0,0,0
	game->camera->setPerspective(70, game->window_width / (float)game->window_height, 0.1, 100000); //set the projection, we want to be perspective

	world = new World();
	world->create();

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse

}

void PlayState::render() {

	Game* game = Game::getInstance();

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	game->camera->set();

	//Draw out world
	//drawGrid(500); //background grid

	world->root->render();

	glDisable(GL_BLEND);

}

void PlayState::update(double seconds_elapsed) {

	Game* game = Game::getInstance();

	double speed = seconds_elapsed * 100; //the speed is defined by the seconds_elapsed so it goes constant

										  //mouse input to rotate the cam
	if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
	{
		game->camera->rotate(game->mouse_delta.x * 0.005, Vector3(0, -1, 0));
		game->camera->rotate(game->mouse_delta.y * 0.005, game->camera->getLocalVector(Vector3(-1, 0, 0)));
	}

	//async input to move the camera around
	if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
	if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) game->camera->move(Vector3(0, 0, 1) * speed);
	if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) game->camera->move(Vector3(0, 0, -1) * speed);
	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) game->camera->move(Vector3(1, 0, 0) * speed);
	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) game->camera->move(Vector3(-1, 0, 0) * speed);

	//to navigate with the mouse fixed in the middle
	if (game->mouse_locked)
	{
		int center_x = floor(game->window_width*0.5);
		int center_y = floor(game->window_height*0.5);

		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = center_x;
		game->mouse_position.y = center_y;
	}

	angle += seconds_elapsed * 10;
}

void PlayState::onLeave(int fut_state) {

}

bool PlayState::isGameOver() {
	return false;
}

void PlayState::reset() {

}