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
#include "../entity.h"
#include "../world.h"
#include "../extra/textparser.h"
#include <algorithm>

#define DEBUG 0

Airplane* eMesh = NULL; // plane mesh
EntityMesh* helix = NULL;
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
	cam3D->setPerspective(70.f, game->window_width / (float)game->window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	// create semi-WORLD

	// plane
	eMesh = new Airplane(SPITFIRE);

	// sky
	bMesh = new EntityMesh();
	bMesh->set("cielo.ASE", "data/textures/cielo.tga", "color");
	bMesh->model.setTranslation(0.f, -500.f, 0.f);

	// ground
	gMesh = new EntityMesh();
	gMesh->set("terrain.ASE", "data/textures/terrain.tga", "color");
	gMesh->model.setTranslation(0.f, -500.f, 0.f);

	// rendering properties

	cam2D.setOrthographic(0.0, game->window_width, game->window_height, 0.0, -1.0, 1.0);
	quad.createQuad(game->window_width * 0.5, game->window_height * 0.95, game->window_width, game->window_height * 0.1, true);

	texture = Texture::Get("data/textures/indications.tga");

	// SET PLANE INFO STRINGS

	std::string filename = "data/game/settings.txt";
	FILE* file = fopen(filename.c_str(), "rb");

	selectionHelp.resize(4);

	for (int i = 0; i < 4; ++i) {
		selectionHelp[i].resize(5);
	}

	selectionHelp[0][0] = "SPITFIRE";
	selectionHelp[1][0] = "P38";
	selectionHelp[2][0] = "WILDCAT";
	selectionHelp[3][0] = "BOMBER";

	TextParser t;
	
	if (!t.create(filename.c_str())) {
		std::cout << "File not found " << filename << std::endl;
		exit(1);
	}

	for (int i = 0; i < 4; i++) {
		t.seek(selectionHelp[i][0].c_str());
		for (int j = 1; j < 5; j++) {
			std::string a(t.getword());
			std::replace(a.begin(), a.end(), '_', ' '); 
			selectionHelp[i][j] = a;
		}
	}
}

void SelectionState::onEnter()
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cout << "$ Entering loading state" << endl;

	playerModel = 0;
	lastRendered = 0;

	//hide the cursor
	SDL_ShowCursor(!game->mouse_locked); //hide or show the mouse
}

void SelectionState::render() {

	//set the clear color (the background color)
	glClearColor(1.0, 1.0, 1.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Put the camera matrices on the stack of OpenGL (only for fixed rendering)
	cam3D->set();

	switch (playerModel) {
	case SPITFIRE:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(SPITFIRE);
		break;
	case P38:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(P38);
		break;
	case WILDCAT:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(WILDCAT);
		break;
	case BOMBER:
		if (lastRendered == playerModel) break;
		eMesh = new Airplane(BOMBER);
		break;
	default:
		break;
	}

	lastRendered = playerModel;
	bMesh->render(cam3D);
	gMesh->render(cam3D);
	eMesh->render(cam3D);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	cam2D.set();
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();

	// PLANE INFO

	for (int i = 0; i < 5; i++) {
		drawText(game->window_width*0.75, game->window_height*0.1*(float)(i+1)/2, selectionHelp[playerModel][i], Vector3(1.f, 1.f, 1.f), 2.0);
	}

	// ************************************************

	drawText(game->window_width*0.1, game->window_height*0.935, "W/S - Change", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(game->window_width*0.4, game->window_height*0.935, "A/D - Move", Vector3(1.f, 1.f, 1.f), 2.0);
	drawText(game->window_width*0.7, game->window_height*0.935, "ENTER - Select", Vector3(1.f, 1.f, 1.f), 2.0);
	
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

}

void SelectionState::update(double time_elapsed) {

	float speed = time_elapsed * 50; //the speed is defined by the seconds_elapsed so it goes constant

	eMesh->update(time_elapsed);

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

	if (state.button[HAT_UP])
	{
		seltimer = 0;
		selectionUp();
	}

	else if (state.button[HAT_DOWN])
	{
		seltimer = 0;
		selectionDown();
	}

	else if (state.button[A_BUTTON])
	{
		seltimer = 0;
		selectionChosen();
	}

	//std:cout << state.axis[LEFT_ANALOG_X] << std::endl;

	if (state.axis[LEFT_ANALOG_X] > 0.2 || state.axis[LEFT_ANALOG_X] < -0.2)
	{
		eMesh->model.rotateLocal(0.05f* speed, Vector3(0, state.axis[LEFT_ANALOG_X], 0));
	}

}

void SelectionState::selectionDown()
{
	s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
	playerModel++;
	if (playerModel == 4) playerModel = 0;
}

void SelectionState::selectionUp()
{
	s_sample = BASS_SampleLoad(false, "data/sounds/move_menu.wav", 0L, 0, 1, 0);
	s_channel = BASS_SampleGetChannel(s_sample, false); // get a sample channel
	BASS_ChannelPlay(s_channel, false); // play it
	playerModel--;
	if (playerModel == -1) playerModel = 3;
}

void SelectionState::selectionChosen()
{
	World* world = World::getInstance();

	// tell world the plane selected
	world->worldInfo.playerModel = playerModel;
	world->addPlayer();
	world->setGameMode();

	SManager->changeCurrentState(PlayState::getInstance(SManager));
}

void SelectionState::onKeyPressed(SDL_KeyboardEvent event)
{
	if (DEBUG) return;

	switch (event.keysym.sym)
	{
	case SDLK_RETURN:
		selectionChosen();
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