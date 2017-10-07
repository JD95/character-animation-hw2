//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// OpenMotionSequenceController.h
//    This is a modified copy of the MotionSequenceController class
//    pulled out of the SKA library to make it more accessible 
//    and hackable for the purpose of the HW02 assignment.
//-----------------------------------------------------------------------------
//    Class OpenMotionSequenceController generates motion by reading 
//    stored poses in a MotionSequence.
//    (A motion sequence is generally read from a .AMC or .BVH mocap file.)
//    Primary function is to convert clock time into a sequence frame.
//-----------------------------------------------------------------------------

#ifndef OPENMOTIONSEQUENCECONTROLLER_DOT_H
#define OPENMOTIONSEQUENCECONTROLLER_DOT_H
#include <Core/SystemConfiguration.h>
#include <Core/Array2D.h>
#include <Math/Matrix4x4.h>
#include <Animation/MotionController.h>
#include <Animation/MotionSequence.h>

class OpenMotionSequenceController : public MotionController
{
public:
	OpenMotionSequenceController() 
		: MotionController(), motion_sequence(NULL), sequence_time(0.0f), sequence_frame(0)
	{ }

	OpenMotionSequenceController(MotionSequence* _ms);
	
	virtual ~OpenMotionSequenceController() { }

	virtual bool isValidChannel(CHANNEL_ID _channel, float _time);

	virtual float getValue(CHANNEL_ID _channel, float _time);

	MotionSequence* getMotionSequence() { return motion_sequence; }
	
	// Functions to access the controller's internal perception of time.
	// This values are both based on state after the last call to getValue().
	// Sequence time is the time within a loop over the motion sequence that
	//   the controller is using.
	// Sequence frame is the actual frame in the motion sequence that was
	//   last accessed.
	float getSequenceTime() { return sequence_time; }
	long getSequenceFrame() { return sequence_frame; }

private:
	MotionSequence* motion_sequence;

	// these two attributes record state at the last call to getValue()
	float sequence_time;	// current (local) time
	long sequence_frame;		// frame accessed for current time

};

#endif // OPENMOTIONSEQUENCECONTROLLER_DOT_H
