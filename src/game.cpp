#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "states/optionsstate.h"
#include "states/menustate.h"
#include "states/playstate.h"
#include "states/selectionstate.h"
#include "states/howto.h"
#include "states/endingstate.h"
#include "states/loadingstate.h"
#include "bass.h"

#include <cmath>

#define DEBUG 0

//some globals
Game* Game::instance = NULL;

// fps
stringstream ss;
double lastTime;
int nbFrames;

Game::Game(SDL_Window* window)
{
	this->window = window;
	instance = this;

	// initialize attributes
	// Warning: DO NOT CREATE STUFF HERE, USE THE INIT 
	// things create here cannot access opengl
	SDL_GetWindowSize( window, &window_width, &window_height );
	std::cout << " * Window size: " << window_width << " x " << window_height << std::endl;

	keystate = NULL;
	joystick = NULL;
	start = false;
	mouse_locked = false;
	music_enabled = true;
	effects_enabled = true; 
	bkg_music_playing = false;
	
	fullscreen = false;
	gameMode = NORMAL;
	ffire_on = false;
	BCK_VOL = 0.5;
}

//Here we have already GL working, so we can create meshes and textures
void Game::init(void)
{
    std::cout << " * Path: " << getPath() << std::endl;
	lastTime = time;
	nbFrames = 0;

	sManager = new StateManager();

	if (DEBUG)
	{
		MenuState::getInstance(sManager)->init();
		SelectionState::getInstance(sManager)->init();
		PlayState::getInstance(sManager)->init();
		sManager->setInitialState(SelectionState::getInstance(sManager));
	}
	else
	{
		sManager->setInitialState(LoadingState::getInstance(sManager));
	}

}

//what to do when the image has to be draw
void Game::render(void)
{
	// render current state
	sManager->render();

	if (DEBUG) {
		// time elapsed
		string stime = "TIME ELAPSED: " + std::to_string(time);
		drawText(5, 5, stime, Vector3(1, 1, 1), 2);

		ss.str("");
		ss << "FPS: " << fps;
		drawText(5, 25, ss.str(), Vector3(1, 1, 1), 2);
	}

	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);
	
}

void Game::update(double seconds_elapsed)
{
	sManager->update( seconds_elapsed );
}

//Keyboard event handler (sync input)
void Game::onKeyPressed( SDL_KeyboardEvent event )
{
	sManager->onKeyPressed( event );

	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: 
			if(!sManager->stateID() || sManager->stateID() == -1)
				exit(1);
			sManager->changeCurrentState( MenuState::getInstance(sManager) );
			break;
	}
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
	sManager->onKeyUp(event);
}


void Game::onMouseButton( SDL_MouseButtonEvent event )
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}

	sManager->onMouseButton(event);
}

void Game::setWindowSize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
    
	/*
    Uint32 flags = SDL_GetWindowFlags(window);
    if(flags & SDL_WINDOW_ALLOW_HIGHDPI)
    {
        width *= 2;
        height *= 2;
    }
	*/

	glViewport( 0,0, width, height );
	//camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

