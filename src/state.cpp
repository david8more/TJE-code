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
void State::onKeyUp(SDL_KeyboardEvent event) {}
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

void StateManager::onKeyUp(SDL_KeyboardEvent event) {
	current_state->onKeyUp(event);
}

void StateManager::OnChar( WPARAM wChar ) {
	current_state->OnChar( wChar );
}

int StateManager::stateID() {
	return current_state->stateID();
}