#include "utils.h"
#include "statemanager.h"
#include "state.h"
#include <cmath>

StateManager::StateManager() : current_state(NULL) {}
StateManager::~StateManager() {}

void StateManager::changeCurrentState(State* new_state) {
	if (current_state) current_state->onLeave(new_state->stateID());
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
	current_state->update(time_elapsed);
}

void StateManager::onKeyPressed(SDL_KeyboardEvent event) {
	current_state->onKeyPressed(event);
}

void StateManager::OnChar(WPARAM wChar) {
	current_state->OnChar(wChar);
}

int StateManager::stateID() {
	return current_state->stateID();
}