#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../soundmanager.h"
#include "../shader.h"
#include "optionsstate.h"
#include "../bass.h"
#include "menustate.h"

#include <cmath>

float timer = 0;

OptionsState::OptionsState(StateManager* SManager) : State( SManager ) {}
OptionsState::~OptionsState() {}

OptionsState* OptionsState::getInstance(StateManager* SManager)
{
	static OptionsState Instance(SManager);
	return &Instance;
}

void OptionsState::init(){
	
	texture = Texture::Get("data/textures/blur.tga");

	game = Game::getInstance();
}

void OptionsState::onEnter()
{
	cout << "$ Entering options state -- ..." << endl;
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.f, 1.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);

	currentSelection = 0;
}

void OptionsState::render() {

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	glEnable(GL_BLEND);
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();
	glDisable(GL_BLEND);

	// menu
	const string submenu_items[] = {"Music", "Music volume", "Effects", "Fullscreen", "Game mode", "Friendly fire", "Back"};

	int N = submenu_items->size();

	int p = 0;
	string enable;

	Vector3 c;
	stringstream ss;
	for(int i = 0; i <= N+1; i++) {
		// highlight current selection
		if(i == currentSelection)
			c = Vector3(1.f, 1.f, 1.f);
		else
			c = Vector3(0.25, 0.25, 0.25);
		switch (i)
		{
		case MUSIC: 
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->music_enabled ? "ENABLED":"DISABLED", c, 3.0);
			break;
		case MUSIC_VOL:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			ss.str("");
			for(int i = 0; i < (int)(game->BCK_VOL * 10); i++)
				ss << "|·|";
			drawText(game->window_width*0.35, game->window_height*0.35 + p, ss.str(), c, 3.0);
			break;
		case EFFECTS: 
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->effects_enabled ? "ENABLED" : "DISABLED", c, 3.0);
			break;
		case FULLSCREEN:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->fullscreen ? "ENABLED" : "DISABLED", c, 3.0);
			break;
		case GAMEMODE:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			if(game->difficulty == D_INSANE)
				drawText(game->window_width*0.35, game->window_height*0.35 + p, "INSANE", c, 3.0);
			else
				drawText(game->window_width*0.35, game->window_height*0.35 + p, game->difficulty == D_BABY ? "BABY" : "SKILLED", c, 3.0);
			break; 
		case FRIENDLYFIRE:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->ffire_on ? "ENABLED" : "DISABLED", c, 3.0);
			break;
		case BACK:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			break;
		}
		p += 40;
	}

	glColor3f(1.f, 1.f, 1.f);
}

void OptionsState::update(double seconds_elapsed) {
	
	Game* game = Game::getInstance();

	if (game->joystick == NULL)
		return;

	JoystickState state = getJoystickState(game->joystick);

	timer += seconds_elapsed;

	if (state.button[HAT_UP] && timer > 0.1)
	{
		selectionUp();
		timer = 0;
	}

	else if (state.button[HAT_DOWN] && timer > 0.1)
	{
		selectionDown();
		timer = 0;
	}

	else if (state.button[A_BUTTON] && timer > 0.2)
	{
		selectionChosen();
		timer = 0;
	}

	else if (state.button[B_BUTTON] && timer > 0.2)
	{
		timer = 0;
		SManager->changeCurrentState(MenuState::getInstance(SManager));
	}

	else if (state.button[HAT_LEFT] && timer > 0.2)
	{
		timer = 0;
		if (currentSelection == MUSIC_VOL)
		{
			downVol();
		}
	}

	else if (state.button[HAT_RIGHT] && timer > 0.2)
	{
		timer = 0;
		if (currentSelection == MUSIC_VOL)
		{
			upVol();
		}
	}

}

void OptionsState::onKeyPressed( SDL_KeyboardEvent event )
{
	switch (event.keysym.sym)
	{
	case SDLK_DOWN:
	case SDLK_s:
		selectionDown();
		break;
	case SDLK_UP:
	case SDLK_w:
		selectionUp();
		break;
	case SDLK_LEFT:
	case SDLK_a:
		if (currentSelection == MUSIC_VOL)
		{
			downVol();
			break;
		}
		else if (currentSelection == GAMEMODE)
		{
			game->difficulty--;
			if (game->difficulty == 0)
				game->difficulty = 3;
			break;
		}
		selectionChosen();
		break;
	case SDLK_RIGHT:
	case SDLK_d:
		if(currentSelection == MUSIC_VOL)
		{	
			upVol();
			break;
		}
		else if (currentSelection == GAMEMODE)
		{
			game->difficulty++;
			if (game->difficulty == 4)
				game->difficulty = 1;
			break;
		}
		selectionChosen();
		break;
	case SDLK_RETURN:
		if(currentSelection == BACK)
			selectionChosen();
		break;
	}
}

void OptionsState::onKeyUp(SDL_KeyboardEvent event)
{
	
}


void OptionsState::onLeave( int fut_state ){
	std::cout << "Options saved correctly" << endl;
}

void OptionsState::upVol()
{
	if(game->BCK_VOL < 1.0)
		game->BCK_VOL += 0.1;
	SoundManager::getInstance()->setVolume("music", game->BCK_VOL);
}

void OptionsState::downVol()
{
	if (game->BCK_VOL > 0.0)
		game->BCK_VOL -= 0.1;
	SoundManager::getInstance()->setVolume("music", game->BCK_VOL);
}

void OptionsState::selectionUp()
{
	currentSelection--;
	if (currentSelection == -1)
		currentSelection = 6;

	if (game->effects_enabled)
		SoundManager::getInstance()->playSound("move_menu", false);
}

void OptionsState::selectionDown()
{
	currentSelection++;
	if (currentSelection == 7)
		currentSelection = 0;

	if (game->effects_enabled)
		SoundManager::getInstance()->playSound("move_menu", false);
}

void OptionsState::selectionChosen()
{
	if (game->effects_enabled)
	{
		SoundManager::getInstance()->playSound("move_menu", false);
	}

	switch (currentSelection)
	{
	case MUSIC:
		game->music_enabled = !game->music_enabled;
		if(!game->music_enabled)
		{
			SoundManager::getInstance()->stopSound("music");
			game->bkg_music_playing = false;
		}
		else 
		{
			SoundManager::getInstance()->playSound("music", true);
			game->bkg_music_playing = true;
		}
		break;
	case EFFECTS:
		game->effects_enabled = !game->effects_enabled;
		break;
	case FULLSCREEN:
		game->fullscreen = !game->fullscreen;
		SDL_SetWindowFullscreen(game->window, game->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP:0  );
   		break;
	case FRIENDLYFIRE:
		game->ffire_on = !game->ffire_on;
		break;
	default: // back to menu
		SManager->changeCurrentState(MenuState::getInstance(SManager));
		break;
	}
}
