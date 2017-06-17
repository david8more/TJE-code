#include "../includes.h"
#include "../camera.h"
#include "../game.h"
#include "../utils.h"
#include "../mesh.h"
#include "../texture.h"
#include "../shader.h"
#include "state.h"
#include "../bass.h"
#include "endingstate.h"
#include "menustate.h"
#include "../world.h"
#include "../extra/textparser.h"

#include <cmath>
#include <iostream>
#include <fstream>

EndingState::EndingState(StateManager* SManager) : State(SManager) {}
EndingState::~EndingState() {}

EndingState* EndingState::getInstance(StateManager* SManager)
{
	static EndingState Instance(SManager);
	return &Instance;
}

void EndingState::onKeyPressed(SDL_KeyboardEvent event)
{
	if(event.keysym.sym == SDLK_RETURN)
	{	
		SManager->changeCurrentState(MenuState::getInstance(SManager));
	}
}

void EndingState::init()
{
	Game * game = Game::instance;

	int w = game->window_width;
	int h = game->window_height;

	texture = Texture::Get("data/textures/ending.tga");

	cam2D.setOrthographic(0.0, w, h, 0.0, -1.0, 1.0);
	quad.createQuad(w * 0.5, h * 0.5, w, h, true);

	// highscores

	TextParser t;

	std::string filename = "data/game/highscores.txt";
	FILE* file = fopen(filename.c_str(), "rb");

	if (!t.create(filename.c_str())) {
		std::cout << "File not found " << filename << std::endl;
		exit(1);
	}

	for (int i = 0; i < 5; i++) {
		t.seek("*SCORE");
		int a = t.getint();
		h_scores.push_back(a);
	}

	NEW_HIGH_SCORE = false;
}

void EndingState::onEnter()
{
	cout << "$ Entering 'ending' state --" << endl;

	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glColor3f(1.f, 1.f, 1.f);
	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Game * game = Game::instance;
	int score = game->score;
	std::stringstream ss;
	int w = game->window_width;
	int h = game->window_height;

	// update highscores with the last score

	if (h_scores[4] > score)
		return;

	std::ofstream myfile("data/game/highscores.txt");
	if (!myfile.is_open())
	{
		std::cout << "no file to write highscores" << endl;
		exit(0);
	}

	for (int i = 0; i < h_scores.size(); i++)
	{
		if (h_scores[i] > score)
		{
			myfile << "*SCORE\n";
			myfile << "\t" << h_scores[i] << "\n\n";
			continue;
		}
		else
		{
			int aux = h_scores[i];
			h_scores[i] = score;
			score = aux;
			NEW_HIGH_SCORE = true;
		}
		myfile << "*SCORE\n";
		myfile << "\t" << h_scores[i] << "\n\n";

	}

	myfile.close();
}

void EndingState::onLeave(int fut_state)
{
	
}

void EndingState::render()
{
	Game * game = Game::instance;
	int score = game->score;

	int w = game->window_width;
	int h = game->window_height;

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// render quad with texture applied
	cam2D.set();
	
	texture->bind();
	quad.render(GL_TRIANGLES);
	texture->unbind();

	glColor3f(1.f, 1.f, 1.f);


	// SCORES

	std::stringstream ss;

	if(NEW_HIGH_SCORE)
		ss << "NEW H ";
	else
		ss << "SCORE: " << game->score;

	drawText(w * 0.5, h * 0.3, ss.str(), Vector3(1.0, 1.0, 1.0), 4.0);

	drawText(w * 0.5, h * 0.4, "HIGH SCORES:", Vector3(1.0, 1.0, 1.0), 3.0);
	
	for (int i = 0; i < h_scores.size(); i++)
	{
		ss.str("");
		ss << h_scores[i];
		drawText(w * 0.525, h * 0.5 + i * h * 0.075, ss.str(), Vector3(1.0, 1.0, 1.0), 3.0);
	}
	

	// 

	ss.str("");
	int diff = game->difficulty;
	std::string rank;
	Texture* insignia_t = Texture::Get("data/textures/insignias/lieutenant.tga");

	switch (diff)
	{
	case D_BABY:

		if (score >= 500) {
			rank = "YOU ARE NOW FIRST LIEUTENANT";
			insignia_t = Texture::Get("data/textures/insignias/lieutenant.tga");
		}

		if (score > 1000) {
			rank = "YOU ARE NOW CAPTAIN";
			insignia_t = Texture::Get("data/textures/insignias/captain.tga");
		}
			
		if (score > 1250) {
			rank = "YOU ARE NOW MAJOR";
			insignia_t = Texture::Get("data/textures/insignias/major.tga");
		}

		if(score > 1500) {
			rank = "YOU ARE NOW COLONEL";
			insignia_t = Texture::Get("data/textures/insignias/colonel.tga");
		}

		break;
	case D_SKILLED:

		if (score > 1625)
			rank = "YOU ARE NOW FIRST LIEUTENANT";
		if (score > 2150)
			rank = "YOU ARE NOW CAPTAIN";
		if (score > 2750)
			rank = "YOU ARE NOW MAJOR";
		if (score > 3250)
			rank = "YOU ARE NOW COLONEL";

		break;
	case D_INSANE:

		if (score > 2600)
			rank = "YOU ARE NOW FIRST LIEUTENANT";
		if (score > 3500)
			rank = "YOU ARE NOW CAPTAIN";
		if (score > 4350)
			rank = "YOU ARE NOW MAJOR";
		if (score > 5250)
			rank = "YOU ARE NOW COLONEL";

		break;
	default:
		break;
	}

	drawText(w * 0.5 - rank.length() * 0.5 * w * 0.03, h * 0.1, rank, Vector3(1.0, 1.0, 1.0), 5.0);

	cam2D.set();

	Mesh insignia;
	insignia.createQuad(w * 0.3, h * 0.5, w * 0.3, h*0.5, true);

	insignia_t->bind();
	insignia.render(GL_TRIANGLES);
	insignia_t->unbind();

	glDisable(GL_BLEND);
}

void EndingState::update(double time_elapsed) {

}
