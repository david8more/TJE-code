#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "optionsstate.h"
#include "../bass.h"
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
	
	texture = Texture::Get("data/textures/blur.tga");

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
	const string submenu_items[] = {"Music", "Effects", "Fullscreen", "Game mode", "Friendly fire", "Back"};

	int p = 0;
	string enable;

	Vector3 c;
	for(int i = 0; i < 6; i++) {
		// highlight current selection
		if(i == currentSelection) c = Vector3(1.f, 1.f, 1.f);
		else c = Vector3(0.25, 0.25, 0.25);
		switch (i)
		{
		case MUSIC: 
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->music_enabled ? "OF COURSE":"NAH", c, 3.0);
			break;
		case EFFECTS: 
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->effects_enabled ? "OF COURSE" : "NAH", c, 3.0);
			break;
		case FULLSCREEN:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->fullscreen ? "OF COURSE" : "NAH", c, 3.0);
			break;
		case GAMEMODE:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, game->gameMode ? "CRUISER" : "HUMAN", c, 3.0);
			break; 
		case FRIENDLYFIRE:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			drawText(game->window_width*0.35, game->window_height*0.35 + p, 0 ? "OF COURSE" : "NAH", c, 3.0);
			break;
		default:
			drawText(game->window_width*0.1, game->window_height*0.35 + p, submenu_items[i], c, 3.0);
			break;
		}
		p += 40;
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
		if (currentSelection != 5) selectionChosen();
		break;
	case SDLK_RIGHT:
	case SDLK_d:
		if (currentSelection != 5) selectionChosen();
		break;
	case SDLK_RETURN:
		if(currentSelection == 5) selectionChosen();
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
		currentSelection = 5;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void OptionsState::selectionDown()
{
	currentSelection++;
	if (currentSelection==6)
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
		else {
			int b_sample = BASS_SampleLoad(false, "data/sounds/lluvia.wav", 0L, 0, 1, BASS_SAMPLE_LOOP);
			b_channel = BASS_SampleGetChannel(b_sample, false); // get a sample channel
			BASS_ChannelPlay(b_channel, false); // play it
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
	case GAMEMODE:
		game->gameMode = !game->gameMode;
		break; 
	case FRIENDLYFIRE:

		break;
	default: // back to menu
		SManager->changeCurrentState(MenuState::getInstance(SManager));
		break;
	}
}
