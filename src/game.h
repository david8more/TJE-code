/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#define D_BABY 1
#define D_SKILLED 2
#define D_INSANE 3

#define DEBUG_TOGAME 1

#include "includes.h"
#include "utils.h"
#include "camera.h"
#include "states/state.h"

class StateManager;

class Game
{
public:
	static Game* instance;
	bool start;
	bool end;
	bool loseWin; // 0 lose 1 win
	bool inGame_DEBUG;

	// SINGLETON
	static Game* getInstance() {
		return instance;
	}

	//window
	SDL_Window* window;
	int window_width;
	int window_height;
    
	int score;

    float time;
	long frame;
	int fps;
	float elapsed_time;

	int difficulty;
	bool ffire_on; // friendly fire

	// State Delegator Instance
	StateManager* sManager;

	// background music playing
	bool bkg_music_playing;
	float BCK_VOL;
	bool music_enabled;
	bool effects_enabled;

	//keyboard state
	const Uint8* keystate;

	//mouse state
	int mouse_state; //tells which buttons are pressed
	Vector2 mouse_position; //last mouse position
	Vector2 mouse_delta; //mouse movement in the last frame
	bool mouse_locked; //tells if the mouse is locked (not seen)

	//joystick

	SDL_Joystick* joystick;
	
	Camera* free_camera; //our global camera
	Camera* fixed_camera;
	Camera* shooter_camera;
	Camera* current_camera;

	bool fullscreen;

	Game(SDL_Window* window);
	void init( void );
	void render( void );
	void update( double dt );

	void onKeyPressed( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButton( SDL_MouseButtonEvent event );
    void onResize( SDL_Event e );

	bool friendlyFireDisabled();
    
	void setWindowSize(int width, int height);
		 
};


#endif 