#include "playercontroller.h"
#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "bass.h"
#include "entity.h"
#include "world.h"
#include "bulletmanager.h"

PlayerController * PlayerController::instance = NULL;

PlayerController::PlayerController()
{
	
}

void PlayerController::setPlayer(Airplane* player)
{
	this->player = player;
}

PlayerController::~PlayerController()
{

}

void PlayerController::update(float seconds_elapsed)
{
	Game* game = Game::getInstance();
	World* world = World::getInstance();

	double speed = seconds_elapsed * 300; //the speed is defined by the seconds_elapsed so it goes constant

	if (game->mouse_locked || (game->mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
	{
		world->playerAir->model.rotateLocal(game->mouse_delta.x * 0.005, Vector3(0, -1, 0));
		world->playerAir->model.rotateLocal(game->mouse_delta.y * 0.005, Vector3(1, 0, 0));
	}

	//async input to move the camera around
	if (game->keystate[SDL_SCANCODE_LSHIFT])
		speed *= 50; //move faster with left shift

	if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP])
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(-1.f, 0.f, 0.f) * speed);

	if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN])
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(1.f, 0.f, 0.f) * speed);

	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT])
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0.f, 0.f, 1.f) * speed);

	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT])
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0.f, 0.f, -1.f) * speed);

	if (game->keystate[SDL_SCANCODE_Q])
	{
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, 1) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, -1, 0) * speed);
	}

	if (game->keystate[SDL_SCANCODE_E])
	{
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 0, -1) * speed);
		world->playerAir->model.rotateLocal(seconds_elapsed, Vector3(0, 1, 0) * speed);
	}

	player->model.traslateLocal(0, 0, speed * seconds_elapsed);
}