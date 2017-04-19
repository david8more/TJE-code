#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "optionsstate.h"
#include "bass.h"
#include "menustate.h"

#include <cmath>

OptionsState::OptionsState(StateManager* SManager) : State( SManager ) {}
OptionsState::~OptionsState() {}

OptionsState* OptionsState::getInstance(StateManager* SManager)
{
	static OptionsState Instance(SManager);
	return &Instance;
}

void OptionsState::init(){
	
	texture = Texture::Get("data/textures/main.tga");

	game = Game::getInstance();
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
}

void OptionsState::onEnter()
{
	cout << "$ Entering options state -- ..." << endl;
	
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.f, 1.f, 1.f);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;

	MenuState* mState = MenuState::getInstance( game->sManager );

	b_channel = mState->b_channel;
}

void OptionsState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	const string submenu_items[] = {"Music", "Effects", "Fullscreen", "Game mode", "back"};

	int p = 0;
	string enable;

	Vector3 c;
	for(int i = 0; i < 5; i++) {
		// highlight current selection
		if(i == currentSelection) c = Vector3(1.f, 1.f, 1.f);
		else c = Vector3(0.15, 0.15, 0.15);
		switch (i)
		{
		case MUSIC: 
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			drawText(275.0,225.0 + p, game->music_enabled ? "YES":"NO", c, 3.0);
			break;
		case EFFECTS: 
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			drawText(275.0, 225.0 + p, game->effects_enabled ? "YES" : "NO", c, 3.0);
			break;
		case FULLSCREEN:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			drawText(275.0, 225.0 + p, game->fullscreen ? "YES" : "NO", c, 3.0);
			break;
		case GAMEMODE:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			drawText(275.0, 225.0 + p, game->gameMode ? "HARD" : "NORMAL", c, 3.0);
			break;
		default:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			break;
		}
		p += 35;
	}

	glColor3f(1.f, 1.f, 1.f);
}

void OptionsState::update(double seconds_elapsed) {
	
	
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
		selectionChosen();
		break;
	case SDLK_RIGHT:
	case SDLK_d:
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

void OptionsState::selectionUp()
{
	currentSelection--;
	if (currentSelection==-1)
		currentSelection = 4;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void OptionsState::selectionDown()
{
	currentSelection++;
	if (currentSelection==5)
		currentSelection = 0;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void OptionsState::selectionChosen()
{
	if (game->effects_enabled)
	{
		s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
	}

	switch (currentSelection)
	{
	case MUSIC:
		game->music_enabled = !game->music_enabled;
		if(game->bkg_music_playing){
			BASS_ChannelStop(b_channel); // stop music 
			game->bkg_music_playing = false;
		}
		break;
	case EFFECTS:
		game->effects_enabled = !game->effects_enabled;
		break;
	case FULLSCREEN:
		game->fullscreen = !game->fullscreen;
		SDL_SetWindowFullscreen(game->window, game->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP:0  );
   		break;
	case GAMEMODE:
		game->gameMode = !game->gameMode;
		break;
	default: // back to menu
		SManager->changeCurrentState(MenuState::getInstance(SManager));
		break;
	}
}
