#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "state.h"
#include "../bass.h"
#include "selectionstate.h"
#include "playstate.h"
#include "menustate.h"
#include "../entity.h"
#include "../world.h"
#include "../extra/textparser.h"
#include "../soundmanager.h"
#include <algorithm>

#define DEBUG 0

Airplane* eMesh = NULL; // plane mesh
Ship* sMesh = NULL; // background ship mesh
EntityMesh* bMesh = NULL; // background mesh
EntityMesh* gMesh = NULL; // ground mesh
Camera* cam3D = NULL;
float seltimer = 0;

SelectionState::SelectionState(StateManager* SManager) : State(SManager) {}
SelectionState::~SelectionState() {}

SelectionState* SelectionState::getInstance(StateManager* SManager)
{
	static SelectionState Instance(SManager);
	return &Instance;
}

void SelectionState::init() {

	//OpenGL flags
	glEnable(GL_CULL_FACE); //render both sides of every triangle
	glEnable(GL_DEPTH_TEST); //check the occlusions using the Z buffer

	// Cogemos la instancia de game para no hacerlo en cada método
	game = Game::getInstance();

	//create our camera
	cam3D = new Camera();
	cam3D->lookAt(Vector3(0.f, 5.f, 15.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 1.f, 0.f)); //position the camera and point to 0,0,0

	// create semi-WORLD

	// plane
	eMesh = new Airplane(SPITFIRE, NULL);

	// sky
	bMesh = new EntityMesh();
	bMesh->set("cielo.ASE", "data/textures/cielo.tga", "simple");
	bMesh->model.setTranslation(0.f, 0.f, 0.f);

	// ground
	gMesh = new EntityMesh();
	gMesh->set("agua.ASE", "data/textures/agua.tga", "water");
	gMesh->model.setTranslation(0.f, -20.f, 0.f);

	sMesh = new Ship(false);
	sMesh->model.setTranslation(-200.f, -10.f, -50.f);
	sMesh->model.rotate(90 * DEG2RAD, Vector3(0, 1, 0));


	texture = Texture::Get("data/textures/indications.tga");

	// SET PLANE INFO STRINGS

	std::string filename = "data/game/settings.txt";
	FILE* file = fopen(filename.c_str(), "rb");

	selectionHelp.resize(3);

	for (int i = 0; i < selectionHelp.size(); ++i) {
		selectionHelp[i].resize(5);
	}

	selectionHelp[0][0] = "SPITFIRE";
	selectionHelp[1][0] = "P38";
	selectionHelp[2][0] = "WILDCAT";

	TextParser t;
	
	if (!t.create(filename.c_str())) {
		std::cout << "File not found " << filename << std::endl;
		exit(1);
	}

	for (int i = 0; i < selectionHelp.size(); i++) {
		t.seek(selectionHelp[i][0].c_str());
		for (int j = 1; j < 5; j++) {
			std::string a(t.getword());
			std::replace(a.begin(), a.end(), '_', ' '); 
			selectionHelp[i][j] = a;
		}
	}

	lastRendered = 0;
}

void SelectionState::onEnter()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cout << "$ Entering loading state" << endl;
	playerModel = 0;
	SDL_ShowCursor(!game->mouse_locked);

	cam3D->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1f, 10000.f); //set the projection, we want to be perspective
	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);

	quad.createQuad(game->window_width * 0.5, game->window_height * 0.95, game->window_width, game->window_height * 0.1);

	if (0 && !DEBUG)
	{
		selectionChosen();
	}
}

void SelectionState::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	cam3D->set();

	switch (playerModel) {
	case SPITFIRE:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(SPITFIRE, false);
		break;
	case P38:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(P38, false);
		break;
	case WILDCAT:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(WILDCAT, false);
		break;
	default:
		break;
	}

	lastRendered = playerModel;
	bMesh->render(cam3D);
	gMesh->render(cam3D);
	sMesh->render(cam3D);
	eMesh->render(cam3D);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();

	// PLANE INFO
	int w = game->window_width;
	int h = game->window_height;

	for (int i = 0; i < 5; i++) {
		drawText(w * 0.75, h * 0.1 * (float)(i+1)/2, selectionHelp[playerModel][i], Vector3(1.f, 1.f, 1.f), 2.0);
	}

	// ************************************************

	drawText(w * 0.1, h * 0.935, "W/S - Change", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(w * 0.4, h * 0.935, "A/D - Move", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(w * 0.7, h * 0.935, "ENTER - Select", Vector3(1.f, 1.f, 1.f), 2.0);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

}

void SelectionState::update(double time_elapsed)
{

	float speed = time_elapsed * 50; //the speed is defined by the seconds_elapsed so it goes constant

	eMesh->model.rotate(-time_elapsed * 0.15, Vector3(0, 1, 0));
	sMesh->model.traslateLocal(0, 0, 22 * speed * -time_elapsed);

	if ((sMesh->model * Vector3()).x > 320.0)
	{
		sMesh->model.setTranslation(-200.f, -10.f, 350.f);
		sMesh->model.rotate(90 * DEG2RAD, Vector3(0, 1, 0));
	}

	if (DEBUG)
	{
		//mouse input to rotate the cam
		if ((game->mouse_state & SDL_BUTTON_LEFT) || game->mouse_locked) //is left button pressed?
		{
			cam3D->rotate(game->mouse_delta.x * 0.005f, Vector3(0.0f, -0.5f, 0.0f));
			cam3D->rotate(game->mouse_delta.y * 0.005f, cam3D->getLocalVector(Vector3(-0.5f, 0.0f, 0.0f)));
		}

		//async input to move the camera around
		if (game->keystate[SDL_SCANCODE_LSHIFT]) speed *= 10; //move faster with left shift
		if (game->keystate[SDL_SCANCODE_W] || game->keystate[SDL_SCANCODE_UP]) cam3D->move(Vector3(0.0f, 0.0f, 0.1f) * speed);
		if (game->keystate[SDL_SCANCODE_S] || game->keystate[SDL_SCANCODE_DOWN]) cam3D->move(Vector3(0.0f, 0.0f, -0.1f) * speed);
		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) cam3D->move(Vector3(0.1f, 0.0f, 0.0f) * speed);
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) cam3D->move(Vector3(-0.1f, 0.0f, 0.0f) * speed);
	}
	else
	{
		if (game->keystate[SDL_SCANCODE_A] || game->keystate[SDL_SCANCODE_LEFT]) eMesh->model.rotateLocal(0.05f* speed, Vector3(0, 1, 0));
		if (game->keystate[SDL_SCANCODE_D] || game->keystate[SDL_SCANCODE_RIGHT]) eMesh->model.rotateLocal(0.05f* speed, Vector3(0, -1, 0));
	}

	//to navigate with the mouse fixed in the middle
	if (game->mouse_locked)
	{
		int center_x = (int)floor(game->window_width*0.5f);
		int center_y = (int)floor(game->window_height*0.5f);

		SDL_WarpMouseInWindow(game->window, center_x, center_y); //put the mouse back in the middle of the screen

		game->mouse_position.x = (float)center_x;
		game->mouse_position.y = (float)center_y;
	}

	// JOYSTICK
	
	seltimer += time_elapsed;

	if (game->joystick == NULL || seltimer < 0.1)
		return;

	JoystickState state = getJoystickState(game->joystick);

	if (state.axis[LEFT_ANALOG_Y] > 0.2 && seltimer > 0.2)
	{
		selectionDown();
		seltimer = 0;
	}

	if (state.axis[LEFT_ANALOG_Y] < -0.2 && seltimer > 0.2)
	{
		selectionUp();
		seltimer = 0;
	}

	if (state.button[A_BUTTON] && seltimer > 0.2)
	{
		selectionChosen();
		seltimer = 0;
	}

	else if (state.button[B_BUTTON] && seltimer > 0.2)
	{
		seltimer = 0;
		SManager->changeCurrentState(MenuState::getInstance(SManager));
	}

	//std:cout << state.axis[LEFT_ANALOG_X] << std::endl;

	// WIN 7
	if (state.axis[TRIGGERS] > 0.2 || state.axis[TRIGGERS] < -0.2)
	{
		eMesh->model.rotateLocal(0.05f* speed, Vector3(0, state.axis[TRIGGERS], 0));
	}

}

void SelectionState::selectionDown()
{
	if(Game::instance->effects_enabled)
		SoundManager::instance->playSound("move_menu", false);
	playerModel++;
	if (playerModel == 3)
		playerModel = 0;
}

void SelectionState::selectionUp()
{
	if (Game::instance->effects_enabled)
		SoundManager::instance->playSound("move_menu", false);
	playerModel--;
	if (playerModel == -1)
		playerModel = 2;
}

void SelectionState::selectionChosen()
{
	World* world = World::getInstance();

	if (Game::instance->end)
		world->reset();

	// tell world the plane selected
	world->worldInfo.playerModel = playerModel;
	world->addPlayer();
	world->addEnemies();
	//world->setGameMode();

	SManager->changeCurrentState(PlayState::getInstance(SManager));
}

void SelectionState::onKeyPressed(SDL_KeyboardEvent event)
{
	if (DEBUG)
		return;

	switch (event.keysym.sym)
	{
	case SDLK_RETURN:
		selectionChosen();
		break;
	case SDLK_3:
		Shader::ReloadAll();
		break;
	case SDLK_UP:
	case SDLK_w:
		selectionUp();
		break;
	case SDLK_DOWN:
	case SDLK_s:
		selectionDown();
		break;
	}
}