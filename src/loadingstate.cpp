#include "camera.h"
#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "rendertotexture.h"
#include "shader.h"
#include "state.h"
#include "bass.h"
#include "loadingstate.h"
#include "menustate.h"
#include "playstate.h"
#include "howto.h"
#include <cmath>
#include <ctime>

LoadingState::LoadingState(StateManager* SManager) : State(SManager) {}
LoadingState::~LoadingState() {}

LoadingState* LoadingState::getInstance(StateManager* SManager)
{
	static LoadingState Instance(SManager);
	return &Instance;
}

void LoadingState::onKeyPressed(SDL_KeyboardEvent event)
{
	SManager->changeCurrentState(MenuState::getInstance(SManager));

	switch (event.keysym.sym)
	{
	
	}
}

void LoadingState::init() {

	// Cargamos texturas de menú
	texture = new Texture();
	if (texture->load("data/textures/loading.tga"))
		cout << "Texture loaded!" << endl;
	else {
		cout << "Error in menu: texture has not been loaded" << endl;
		exit(1);
	}

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	// configuración inicial
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.5, game->window_width, game->window_height, true);

	loaded = false;
	changing_dot = 0;
}

void LoadingState::onEnter()
{
	if (!loaded) return;

	cout << "$ Entering loading state -- ..." << endl;

	MenuState::getInstance(this->SManager)->init();
	OptionsState::getInstance(this->SManager)->init();
	Howto::getInstance(this->SManager)->init();
	PlayState::getInstance(this->SManager)->init();
}

void LoadingState::render() {

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	// render quad with texture applied
	cam2D.set();
	glEnable(GL_BLEND);
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();
	glDisable(GL_BLEND);

	if (!loaded) {
		loaded = true;
		onEnter();
	}

	glColor3f(1.0, 1.0, 1.0);

	dots.vertices.resize(10);
	dots.colors.resize(10);

	int N = 30;
	int j = 0;
	for (int i = 0; i <= 360; i += (360 / 8)) {
		dots.vertices[j] = (Vector3(game->window_width / 2 + cos(DEG2RAD*i)* N, game->window_height / 1.5 + sin(DEG2RAD*i)* N, 0.0));
		j++;
		dots.colors[changing_dot] = (Vector4(1, 0, 0, 0));
	}	

	dots.render(GL_POINTS);
	dots.colors.clear();
}

void LoadingState::update(double time_elapsed) {

	changing_dot++;
	if (changing_dot > 8) changing_dot = 0;

}