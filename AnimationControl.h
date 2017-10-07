//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// AnimationControl.h
//    Animation controller for multiple characters.
//    This is intended to be used for HW2 - COMP 259 fall 2017.
//-----------------------------------------------------------------------------
#ifndef ANIMATIONCONTROL_DOT_H
#define ANIMATIONCONTROL_DOT_H
// SKA configuration
#include <Core/SystemConfiguration.h>
// C/C++ libraries
#include <list>
#include <vector>
using namespace std;
// SKA modules
#include <Objects/Object.h>

class Skeleton;

struct AnimationControl
{
private:
	// state for basic functionality
	bool ready;
	float run_time;
	vector<Skeleton*> characters;

	// state for enhanced functionality
	float global_timewarp;
	float next_marker_time;
	float marker_time_interval;
	float max_marker_time;

public:
	AnimationControl();
	virtual ~AnimationControl();

	// loadCharacters() sets up the characters and their motion control.
	// It places all the bone objects for each character into the render list,
	// so that they can be drawn by the graphics subsystem.
	void loadCharacters();

	// updateAnimation() should be called every frame to update all characters.
	// _elapsed_time should be the time (in seconds) since the last frame/update.
	bool updateAnimation(float _elapsed_time);

	bool isReady() { return ready; }

	float getRunTime() { return run_time; }

	// restart resets everything to time = 0
	void restart();

	void increaseGlobalTimeWarp() { global_timewarp *= 2.0f; }
	void decreaseGlobalTimeWarp() { global_timewarp /= 2.0f; }
	float getGlobalTimeWarp() { return global_timewarp;  }
};

// global single instance of the animation controller
extern AnimationControl anim_ctrl;

#endif // ANIMATIONCONTROL_DOT_H
