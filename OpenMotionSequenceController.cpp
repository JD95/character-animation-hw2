//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// OpenMotionSequenceController.cpp
//    This is a modified copy of the MotionSequenceController class
//    pulled out of the SKA library to make it more accessible 
//    and hackable for the purpose of the HW02 assignment.
//-----------------------------------------------------------------------------
//    Class OpenMotionSequenceController generates motion by reading 
//    stored poses in a MotionSequence.
//    (A motion sequence is generally read from a .AMC or .BVH mocap file.)
//    Primary function is to convert clock time into a sequence frame.
//-----------------------------------------------------------------------------

#include <Core/SystemConfiguration.h>
#include <Core/Utilities.h>
#include <Animation/AnimationException.h>
#include "OpenMotionSequenceController.h"

OpenMotionSequenceController::OpenMotionSequenceController(MotionSequence* _ms) 
	: MotionController(), motion_sequence(_ms), sequence_time(0.0f), sequence_frame(0)
{ 
}

bool OpenMotionSequenceController::isValidChannel(CHANNEL_ID _channel, float _time)
{	
	if (motion_sequence == NULL) 
	{
		throw AnimationException("OpenMotionSequenceController has no attached MotionSequence");
		return false;
	}
	return motion_sequence->isValidChannel(_channel);
}

float OpenMotionSequenceController::getValue(CHANNEL_ID _channel, float _time)
{
	if (motion_sequence == NULL) 
		throw AnimationException("OpenMotionSequenceController has no attached MotionSequence");

	if (!isValidChannel(_channel, _time)) 
	{
		string s = string("OpenMotionSequenceController received request for invalid channel ") 
			+ " bone: " + toString(_channel.bone_id) + " dof: " + toString(_channel.channel_type);
		throw AnimationException(s.c_str());
	}

	float duration = motion_sequence->getDuration();
	long cycles = long(_time / duration);
	
	sequence_time = _time - duration*cycles;
	if (sequence_time > duration) sequence_time = 0.0f;

	int frame = int(motion_sequence->numFrames()*sequence_time/duration);
	sequence_frame = frame;

	float value = motion_sequence->getValue(_channel, frame);

	return value;
}
