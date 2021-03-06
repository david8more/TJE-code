#include "playercontroller.h"
#include "camera.h"
#include "game.h"
#include "world.h"
#include "states\playstate.h"
#include "soundmanager.h"

PlayerController * PlayerController::instance = NULL;
float controller_timer = 0;

PlayerController::PlayerController()
{
	current_controller = CONTROLLER_MODE_KEYBOARD;
}

PlayerController::~PlayerController()
{

}

void PlayerController::update(float seconds_elapsed)
{
	Game* game = Game::getInstance();

	// controlled changed?

	if (game->joystick != NULL)
	{
		if (getJoystickState(game->joystick).button[BACK_BUTTON])
		{
			current_controller = CONTROLLER_MODE_GAMEPAD;
			return;
		}

		if (getJoystickState(game->joystick).button[START_BUTTON])
		{
			current_controller = CONTROLLER_MODE_GAMEPAD;
			game->start = true;
			game->current_camera->near_plane = 7.5f;
			SoundManager::getInstance()->playSound("plane", true);
		}
	}

	

	// GAME STARTED?
	
	if (!game->start)
		return;

	PlayState* playstate = PlayState::getInstance(game->sManager);

	// CONTROLLER

	double speed = player->speed;

	if (current_controller == CONTROLLER_MODE_KEYBOARD)
	{
		if (game->mouse_locked || (game->mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
		{
			player->model.rotateLocal(game->mouse_delta.x * 0.005, Vector3(0, -1, 0));
			player->model.rotateLocal(game->mouse_delta.y * 0.005, Vector3(1, 0, 0));
		}

		//async input to move the camera around
		if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 2; //move faster with left shift
		//if (game->keystate[SDL_SCANCODE_RSHIFT]) speed *= 50; //move mega faster with right shift

		if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP])
			moveY(-1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN])
			moveY(1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT])
			moveX(1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT])
			moveX(-1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_Q])
			moveXY(1.f, -1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_E])
			moveXY(-1.f, 1.f, seconds_elapsed, speed);

		if (game->keystate[SDL_SCANCODE_SPACE])
			player->shoot();
		if (game->keystate[SDL_SCANCODE_R])
			player->rear_shoot();

		// to navigate with the mouse fixed in the middle
		if (game->mouse_locked)
		{
			int center_x = floor(game->window_width*0.5);
			int center_y = floor(game->window_height*0.5);

			SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

			game->mouse_position.x = center_x;
			game->mouse_position.y = center_y;
		}
	}
	else if(current_controller == CONTROLLER_MODE_GAMEPAD)
	{
		// JOYSTICK

		if (game->joystick == NULL)
			return;

		JoystickState state = getJoystickState(game->joystick);

		controller_timer += seconds_elapsed;

		if (state.axis[LEFT_ANALOG_Y] > 0.2 || state.axis[LEFT_ANALOG_Y] < -0.2)
		{
				moveY(state.axis[LEFT_ANALOG_Y], seconds_elapsed, speed);
		}

		if (state.axis[RIGHT_ANALOG_X] > 0.2 || state.axis[RIGHT_ANALOG_X] < -0.2)
		{
			// Q, E on keyboard
			 moveXY(-state.axis[RIGHT_ANALOG_X], state.axis[RIGHT_ANALOG_X], seconds_elapsed, speed);
			//
			//moveX(-state.axis[RIGHT_ANALOG_X], seconds_elapsed, speed);
		}

		if (state.button[HAT_LEFT])
		{
			moveX(1.f, seconds_elapsed, speed);
		}

		if (state.button[HAT_RIGHT])
		{
			moveX(-1.f, seconds_elapsed, speed);
		}

		if (state.button[Y_BUTTON] && controller_timer > 0.25)
		{
			playstate->current_view++;
			if (playstate->current_view > SHOOTERVIEW)
				playstate->current_view = FULLVIEW;
			playstate->setView();
			controller_timer = 0;
		}

		if (state.button[RB_BUTTON] && controller_timer > 0.25)
		{
			playstate->inZoom = !playstate->inZoom;
			playstate->setZoom();
			controller_timer = 0;
		}

		if (state.button[LB_BUTTON] && controller_timer > 0.25)
		{
			player->torpedoShoot();
			controller_timer = 0;
		}

		// COOPERATIVE
		if (game->keystate[SDL_SCANCODE_R])
			player->rear_shoot();
		
		// WIN 10
		if (state.axis[LEFT_TRIGGER] > 0.2)
		{
			speed *= 2;
		}

		if (state.axis[RIGHT_TRIGGER] > 0.2)
		{
			player->shoot();
		}
		else {
			player->shootingtime = 0;
		}

		// WIN 7
		/*if (state.axis[TRIGGERS] > 0.2)
		{
			speed *= 2;
		}

		if (state.axis[TRIGGERS] < -0.2)
		{
			player->shoot();
		}
		else {
			player->shootingtime = 0;
		}*/
		
	}

	float playTime = PlayState::getInstance(Game::instance->sManager)->playTime;

	if (playTime < 1.0)
		player->model.traslateLocal(0, 0, speed * seconds_elapsed * pow(playTime, 5));
	else
		player->model.traslateLocal(0, 0, speed * seconds_elapsed);

	
}

void PlayerController::moveY(float axis, float seconds_elapsed, float speed)
{
	player->model.rotateLocal(seconds_elapsed, Vector3(axis, 0.f, 0.f) * speed);
}

void PlayerController::moveX(float axis, float seconds_elapsed, float speed)
{
	player->model.rotateLocal(seconds_elapsed, Vector3(0.f, 0.f, axis) * speed);
}

void PlayerController::moveXY(float Zaxis, float Yaxis, float seconds_elapsed, float speed)
{
	player->model.rotateLocal(seconds_elapsed, Vector3(0, 0, Zaxis) * speed);
	player->model.rotateLocal(seconds_elapsed * 0.3, Vector3(0, Yaxis, 0) * speed);
}

void PlayerController::updateCamera(Camera * camera, float seconds_elapsed)
{
	Game* game = Game::getInstance();
	double speed = seconds_elapsed * 50; //the speed is defined by the seconds_elapsed so it goes constant

	if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 50;
	if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) game->free_camera->move(Vector3(0.f, 0.f, 1.f) * speed);
	if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) game->free_camera->move(Vector3(0.f, 0.f, -1.f) * speed);
	if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) game->free_camera->move(Vector3(1.f, 0.f, 0.f) * speed);
	if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) game->free_camera->move(Vector3(-1.f, 0.f, 0.f) * speed);

	if (game->mouse_locked || (game->mouse_state & SDL_BUTTON_LEFT)) //is left button pressed?
	{
		game->free_camera->rotate(game->mouse_delta.x * 0.005f, Vector3(0.f, -1.f, 0.f));
		game->free_camera->rotate(game->mouse_delta.y * 0.005f, game->current_camera->getLocalVector(Vector3(-1.f, 0.f, 0.f)));
	}

	if (game->mouse_locked)
	{
		int center_x = floor(game->window_width*0.5);
		int center_y = floor(game->window_height*0.5);

		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = center_x;
		game->mouse_position.y = center_y;
	}
}