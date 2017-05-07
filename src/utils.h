/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This contains several functions that can be useful when programming your game.
*/
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>
#include <vector>

#include "framework.h"
#include "includes.h"

//General functions **************
long getTime();

//generic purposes fuctions
void drawGrid(float dist);
bool drawText(float x, float y, std::string text, Vector3 c, float scale = 1);

//check opengl errors
bool checkGLErrors();

std::string getPath();

Vector2 getDesktopSize( int display_index = 0 );


std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

//mapped as in SDLs

enum XBOXpad
{
	//axis
	LEFT_ANALOG_X = 0,
	LEFT_ANALOG_Y = 1,
	RIGHT_ANALOG_X = 2,
	RIGHT_ANALOG_Y = 3,
	LEFT_TRIGGER = 4, //both triggers share an axis (positive is right, negative is left trigger)
	RIGHT_TRIGGER = 5,

				  //buttons
				  A_BUTTON = 10,
				  B_BUTTON = 11,
				  X_BUTTON = 12,
				  Y_BUTTON = 13,
				  LB_BUTTON = 8,
				  RB_BUTTON = 9,
				  BACK_BUTTON = 5,
				  START_BUTTON = 4,
				  LEFT_ANALOG_BUTTON = 6,
				  RIGHT_ANALOG_BUTTON = 7
};

enum HATState
{
	HAT_CENTERED = 99,
	HAT_UP = 0,
	HAT_RIGHT = 3,
	HAT_DOWN = 1,
	HAT_LEFT = 2,
	HAT_RIGHTUP = (HAT_RIGHT | HAT_UP),
	HAT_RIGHTDOWN = (HAT_RIGHT | HAT_DOWN),
	HAT_LEFTUP = (HAT_LEFT | HAT_UP),
	HAT_LEFTDOWN = (HAT_LEFT | HAT_DOWN)
};

struct JoystickState
{
	int num_axis;	//num analog sticks
	int num_buttons; //num buttons
	float axis[8]; //analog stick
	char button[16]; //buttons
	HATState hat; //digital pad
};

SDL_Joystick* openJoystick(int num_joystick);

JoystickState getJoystickState(SDL_Joystick* joystick);


#endif
