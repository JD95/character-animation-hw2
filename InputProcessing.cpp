//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// InputProcessing.cpp
//    Object to monitor user input from mouse and keyboard.
//-----------------------------------------------------------------------------
// SKA configuration
#include <Core/SystemConfiguration.h>
// SKA modules
#include <Input/InputManager.h>
#include <Input/InputFilter.h>
// local application
#include "InputProcessing.h"
#include "CameraControl.h" 
#include "AnimationControl.h"

InputProcessor input_processor;

// extern references from AppMain.cpp
extern void shutDown(int _exit_code);

#define ESC 27 // ASCII code for the escape key.

// Control Scheme:
// left mouse button: move camera forward
// right mouse button: move camera backward

// This could be an attribute of class InputProcessor, but putting 
// it hear hides it from the outside this code. Since there should
// only be one InputProcessor, this works fine.
static InputFilter* filter = NULL;

InputProcessor::InputProcessor()
{
	// Setup to filter keys that shouldn't get multiple responses 
	// if they are held down too long. Ignore repeated key 
	// responses within 0.2 seconds after another response.
	if (filter == NULL) filter = new InputFilter;
	filter->addFilter('8', 0.2f, KEYBOARD);
	filter->addFilter('9', 0.2f, KEYBOARD);
	filter->addFilter(',', 0.2f, KEYBOARD);
	filter->addFilter('.', 0.2f, KEYBOARD);
}

InputProcessor::~InputProcessor()
{ if (filter == NULL) delete filter; }

void InputProcessor::processInputs(float elapsed_time)
{ 
	// tell input filter how much time has passed
	if (filter != NULL) filter->advanceTime(elapsed_time);

	// get all input activity since last call
	InputActions* actions = input_manager.getInput();

	// initialize camera controls
	float fwd_thrust=0.0f;
	float hrz_thrust=0.0f;
	float vrt_thrust=0.0f;
	float yaw_thrust=0.0f;
	float pitch_thrust=0.0f;
	float roll_thrust=0.0f;
	bool move_camera = false;

	// left and right mouse buttons move camera forward and backward
	if (actions->mouse_button_state[0]) { fwd_thrust += 0.1f; move_camera= true; }
	if (actions->mouse_button_state[1]) { fwd_thrust -= 0.1f; move_camera= true; }

	// check everything in the keyboard queue
	for (short i=0; i<actions->num_keys_pressed; i++)
	{
		unsigned char key;
		if (filter != NULL) key = filter->testInput(actions->keys_pressed[i]);
		else  key = actions->keys_pressed[i];

		switch (key)
		{
		// system controls
		case ESC: shutDown(0); break;
		// camera controls
		case 'w': fwd_thrust += 0.9f;    move_camera= true; break;
		case 's': fwd_thrust -= 0.9f;    move_camera= true; break;
		case 'q': vrt_thrust += 0.9f;    move_camera= true; break;
		case 'e': vrt_thrust -= 0.9f;    move_camera= true; break;
		case 'a': hrz_thrust += 0.9f;    move_camera= true; break;
		case 'd': hrz_thrust -= 0.9f;    move_camera= true; break;
		case 'i': pitch_thrust += 0.05f; move_camera= true;  break;
		case 'k': pitch_thrust -= 0.05f; move_camera= true; break;
		case 'j': yaw_thrust += 0.05f;   move_camera= true; break;
		case 'l': yaw_thrust -= 0.05f;   move_camera= true; break;
		case 'u': roll_thrust += 0.05f;  move_camera= true; break;
		case 'o': roll_thrust -= 0.05f;  move_camera= true; break;
		case '8': 
		{
			short camera_preset = camera.getCameraPreset();
			if (camera_preset == 0) camera_preset = 7;
			else camera_preset -= 1;
			camera.setCameraPreset(camera_preset);
			break;
		}
		case '9': 
		{
			short camera_preset = camera.getCameraPreset();
			camera_preset = (camera_preset + 1) % 8;
			camera.setCameraPreset(camera_preset);
			break;
		}
		case '1':
			anim_ctrl.restart();
			break;
		case ',':
			anim_ctrl.decreaseGlobalTimeWarp();
			break;
		case '.':
			anim_ctrl.increaseGlobalTimeWarp();
			break;
		}
	}
	if (move_camera)
	{
		camera.move(0.02f, fwd_thrust, hrz_thrust, vrt_thrust, yaw_thrust, pitch_thrust, roll_thrust);
	}
}
