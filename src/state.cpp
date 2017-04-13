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
#include "menustate.h"

#include <cmath>

State::State() {}
State::State(StateManager* SManager) : SManager( SManager ) {}
State::~State() {}

void State::changeCurrentState( State* new_state ) {
	SManager->changeCurrentState( new_state );
}

void State::init() {}
void State::render() {}
void State::update(double t) {}

void State::onKeyPressed( SDL_KeyboardEvent event ) {}
void State::onKeyDown(SDL_KeyboardEvent event) {}
void State::OnChar( WPARAM wChar ) {}
int State::stateID() { return -1; }

void State::onEnter() {}
void State::onLeave( int fut_state ) {}

// *********************************************************************************************************
// *********************************************************************************************************

StateManager::StateManager() : current_state( NULL ) {}
StateManager::~StateManager() {}

void StateManager::setInitialState(State* state) {
	current_state = state;
	current_state->onEnter();
}

void StateManager::changeCurrentState( State* new_state ) {
	if( current_state ) current_state->onLeave( new_state->stateID() );
	current_state = new_state;
	current_state->onEnter();
}

void StateManager::init() {
	current_state->init();
}

void StateManager::render() {
	current_state->render();
}

void StateManager::update(double time_elapsed) {
	current_state->update( time_elapsed );
}

void StateManager::onKeyPressed( SDL_KeyboardEvent event ) {
	current_state->onKeyPressed( event );
}

void StateManager::onKeyDown(SDL_KeyboardEvent event) {
	current_state->onKeyDown(event);
}

void StateManager::OnChar( WPARAM wChar ) {
	current_state->OnChar( wChar );
}

int StateManager::stateID() {
	return current_state->stateID();
}

// *********************************************************************************************************
// *********************************************************************************************************

OptionsState::OptionsState(StateManager* SManager) : State( SManager ) {}
OptionsState::~OptionsState() {}

OptionsState* OptionsState::getInstance(StateManager* SManager)
{
	static OptionsState Instance(SManager);
	return &Instance;
}

void OptionsState::onKeyPressed( SDL_KeyboardEvent event )
{
	switch (event.keysym.sym)
	{
	case SDLK_DOWN:
		selectionDown();
		break;
	case SDLK_UP:
		selectionUp();
		break;
	case SDLK_RETURN:
		selectionChosen();
		break;
	}
}

void OptionsState::onKeyDown(SDL_KeyboardEvent event)
{
	
}

void OptionsState::init(){
	
	texture = new Texture();
	if (!texture->load("data/textures/optionsv1.tga")){
		cout << "Error: texture has not been loaded" << endl;
		exit(1);
	}

	game = Game::getInstance();
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);
}

void OptionsState::onEnter()
{
	cout << "$ Entering options state -- ..." << endl;
	
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentSelection = 0;
	
	//Inicializamos BASS  (id_del_device, muestras por segundo, ...)
	BASS_Init(-1, 44100, BASS_DEVICE_DEFAULT, 0, NULL);

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

	// menu
	const string submenu_items[] = {"Music", "Effects", "Fullscreen", "back"};

	int p = 0;
	string enable;

	Vector3 c;
	for(int i = 0; i < 4; i++) {
		// highlight current selection
		if(i == currentSelection) c = Vector3(0.8, 0.0, 0.0);
		else c = Vector3(0.8, 0.8, 0.0);
		switch (i)
		{
		case 0: 
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			enable = valueIfBool(game->music_enabled, "YES", "NO");
			drawText(250.0,225.0 + p, enable, c, 3.0);
			break;
		case 1: 
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			enable = valueIfBool(game->effects_enabled, "YES", "NO");
			drawText(250.0, 225.0 + p, enable, c, 3.0);
			break;
		case 2:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			enable = valueIfBool(game->fullscreen, "YES", "NO");
			drawText(250.0, 225.0 + p, enable, c, 3.0);
			break;
		default:
			drawText(75.0, 225.0 + p, submenu_items[i], c, 3.0);
			break;
		}
		p += 35;
	}

	glDisable(GL_BLEND);
}

void OptionsState::update(double seconds_elapsed) {
	
	
}

void OptionsState::onLeave( int fut_state ){
	std::cout << "Options saved correctly" << endl;
}

void OptionsState::selectionUp()
{
	currentSelection--;
	if (currentSelection==-1)
		currentSelection = 3;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul2.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void OptionsState::selectionDown()
{
	currentSelection++;
	if (currentSelection==4)
		currentSelection = 0;

	if (!game->effects_enabled)
		return;

	s_sample = BASS_SampleLoad(false, "data/sounds/move_menul2.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
}

void OptionsState::selectionChosen()
{
	cout << currentSelection << endl;
	if (game->effects_enabled)
	{
		s_sample = BASS_SampleLoad(false, "data/sounds/sel_menul.wav", 0L, 0, 1, 0);
		s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
		BASS_ChannelPlay(s_channel, false); // play it
	}

	switch (currentSelection)
	{
	case 0: // Music
		game->music_enabled = !game->music_enabled;
		if(game->bkg_music_playing){
			BASS_ChannelStop(b_channel); // stop music 
			game->bkg_music_playing = false;
		}
		break;
	case 1: // Effects
		game->effects_enabled = !game->effects_enabled;
		break;
	case 2: // option 3
		game->fullscreen = !game->fullscreen;
		SDL_SetWindowFullscreen(game->window, game->fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP:0  );
   		break;
	case 3: // back to menu
		SManager->changeCurrentState( MenuState::getInstance(SManager) );
		break;
	}
}
