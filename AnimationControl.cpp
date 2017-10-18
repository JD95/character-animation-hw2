//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// AnimationControl.cpp
//    Animation controller for multiple characters.
//    This is intended to be used for HW2 - COMP 259 fall 2017.
//-----------------------------------------------------------------------------
// SKA configuration
#include <Core/SystemConfiguration.h>
// C/C++ libraries
#include <cstdio>
#include <complex>
#include <algorithm>
// SKA modules
#include <Core/Utilities.h>
//#include <Animation/RawMotionController.h>
#include <Animation/AnimationException.h>
#include <Animation/Skeleton.h>
#include <DataManagement/DataManager.h>
#include <DataManagement/DataManagementException.h>
#include <DataManagement/BVH_Reader.h>
// local application
#include "AppConfig.h"
#include "AnimationControl.h"
#include "RenderLists.h"
#include "OpenMotionSequenceController.h"

// global single instance of the animation controller
AnimationControl anim_ctrl;

enum MOCAP_TYPE { BVH, AMC };

bool timeHasReset = false;
bool doneGettingData = false;

struct LoadSpec {
	MOCAP_TYPE mocap_type;
	float scale;
	Color color;
	string motion_file;
	string skeleton_file;
	LoadSpec(MOCAP_TYPE _mocap_type, float _scale, Color& _color, string& _motion_file, string& _skeleton_file = string(""))
		: mocap_type(_mocap_type), scale(_scale), color(_color), motion_file(_motion_file), skeleton_file(_skeleton_file) { }
};

const short NUM_CHARACTERS = 3;
LoadSpec load_specs[NUM_CHARACTERS] = {
	LoadSpec(BVH, 0.2f, Color(0.0f,1.0f,0.0f), string("take1.bvh")),
	LoadSpec(BVH, 0.2f, Color(0.8f,0.4f,0.8f), string("take2.bvh")),
	LoadSpec(BVH, 0.2f, Color(1.0f,0.4f,0.3f), string("take3.bvh"))
};

Object* createMarkerBox(Vector3D position, Color _color)
{
	ModelSpecification markerspec("Box", _color);
	markerspec.addSpec("length", "0.5");
	markerspec.addSpec("width", "0.5");
	markerspec.addSpec("height", "0.5");
	Object* marker = new Object(markerspec, position, Vector3D(0.0f, 0.0f, 0.0f));
	return marker;
}

AnimationControl::AnimationControl()
	: ready(false), run_time(0.0f),
	global_timewarp(1.0f),
	next_marker_time(0.1f), marker_time_interval(0.1f), max_marker_time(20.0f)
{ }

AnimationControl::~AnimationControl()
{
	for (unsigned short c = 0; c<characters.size(); c++)
		if (characters[c] != NULL) delete characters[c];
}

void AnimationControl::restart()
{
	render_lists.eraseErasables();
	run_time = 0;
	updateAnimation(0.0f);
	next_marker_time = marker_time_interval;
}


void distance_for_frame(vector<MotionData>& data, const unsigned long n) {
	if (n == 0) return;
	data[n].distance = data[n].position - data[n - 1].position;
};

void velocity_for_frame(vector<MotionData>& data, const unsigned long n, const float time) {
	auto x = data[n].distance.x;
	auto y = data[n].distance.y;
	auto z = data[n].distance.z;

	auto d = sqrt(x * x) + (y * y) + (z * z);
	auto vel = (data[n].distance / d) * time;
	data[n].velocity = vel;
};

vector<float> extract_sync_frames(const vector<MotionData>& data) {

	vector<float> sync_frames;
	auto start = data.begin();
	
	bool goingUp = true;

	while (start != data.end()) {
		if (goingUp) {

			start = std::adjacent_find(start, data.end(), [](MotionData prev, MotionData current) {
				return current.distance.getY() < prev.distance.getY();
			});

			goingUp = false;
		}
		else {

			start = std::adjacent_find(start, data.end(), [](MotionData prev, MotionData current) {
				return prev.distance.getY() < current.distance.getY();
			});

			sync_frames.push_back(start->time);
			goingUp = true;
		}
	}
	
	return sync_frames;
}


// figures out what the starting index is
int getStartingIndex(float yRealTime,const vector<float>& y) {
	try {
		for (int i = 0; i < y.size(); i++) {
			if (yRealTime >= y[i] && yRealTime < y[i + 1]) {
				return i;
			}
		}
	}
	catch (int i) {
		std::cout << "Y array does not contain any two values greater than and less than " << yRealTime << endl;
	}
}

// x is being warpped to fit y
float timeWarp(const vector<float>& x,const vector<float>& y, float yRealTime) {
	int startIndex = getStartingIndex(yRealTime, y);
	float xRealTime = 0;
	try {
		xRealTime = (x[startIndex] + (yRealTime - y[startIndex])  * ((x[startIndex + 1] - x[startIndex]) / (y[startIndex + 1] - y[startIndex])));
	}
	catch (int i) {
		std::cout << "Going out of index when accessing index " << i + 1;
	}
	return xRealTime;
}

bool AnimationControl::getData(float _elapsed_time) {
	// the global time warp can be applied directly to the elapsed time between updates
	float warped_elapsed_time = global_timewarp * (1.0/120.0);
	if (!ready) return false;

	for (auto& character : foot_data) {
		 if (character.cycles > 0 && character.sync_frames.size() == 0) {
			for (int i = 1; i < character.motion.size(); i++)
			{
				distance_for_frame(character.motion, i);
				velocity_for_frame(character.motion, i, warped_elapsed_time);
			}

			// Extract Sync Frames
			character.motion.shrink_to_fit();
			character.sync_frames = extract_sync_frames(character.motion);
			std::cout << character.sync_frames.size() << endl;
		}
	}
	if (foot_data[1].cycles > 0) {
		doneGettingData = true;
		run_time = 0;
	}

	run_time += warped_elapsed_time;

	for (unsigned short c = 0; c < 3; c++)
	{
		if (characters[c] != NULL) {
			characters[c]->update(run_time);
		}

		// pull local time and frame out of each skeleton's controller
		// (dangerous upcast)
		OpenMotionSequenceController* controller = (OpenMotionSequenceController*)characters[c]->getMotionController();
		display_data.sequence_time[c] = controller->getSequenceTime();
		display_data.sequence_frame[c] = controller->getSequenceFrame();

		if (display_data.sequence_frame[c] < foot_data[c].prev_frame) {
			foot_data[c].cycles++;
		}

		foot_data[c].prev_frame = display_data.sequence_frame[c];
	}

	// Record foot position until a complete
	// cycle through the animation
	for (unsigned short c = 0; c < characters.size(); c++)
	{
		if (foot_data[c].cycles == 0) {
			Vector3D start, end;

			// drop box at left toes of 1st character
			// CAREFUL - bones names are different in different skeletons
			characters[c]->getBonePositions("LeftToeBase", start, end);
			foot_data[c].motion.emplace_back(end, display_data.sequence_frame[c], run_time);
		}
	}


	if (run_time >= next_marker_time && run_time <= max_marker_time)
	{
		Color color = Color(0.8f, 0.3f, 0.3f);
		Vector3D start, end;
		// drop box at left toes of 1st character
		// CAREFUL - bones names are different in different skeletons
		characters[0]->getBonePositions("LeftToeBase", start, end);
		Object* marker = createMarkerBox(end, color);
		render_lists.erasables.push_back(marker);
		next_marker_time += marker_time_interval;
	}
}

bool AnimationControl::warpTime(float _elapsed_time) {

	// the global time warp can be applied directly to the elapsed time between updates
	float warped_elapsed_time = global_timewarp * _elapsed_time;
	if (!ready) return false;

	run_time += warped_elapsed_time;

	for (auto& character : foot_data) {
		if (run_time > character.sync_frames[character.sync_frames.size() - 1])
			run_time = 0;
	}

	for (unsigned short c = 0; c < 3; c++)
	{
		if (characters[c] != NULL) {
			if (c != 0) {
				characters[c]->update(timeWarp(foot_data[0].sync_frames, foot_data[c].sync_frames, run_time));
			}
			else {
				characters[c]->update(run_time);
			}
		}

		// pull local time and frame out of each skeleton's controller
		// (dangerous upcast)
		OpenMotionSequenceController* controller = (OpenMotionSequenceController*)characters[c]->getMotionController();
		display_data.sequence_time[c] = controller->getSequenceTime();
		display_data.sequence_frame[c] = controller->getSequenceFrame();

		if (display_data.sequence_frame[c] < foot_data[c].prev_frame) {
			foot_data[c].cycles++;
		}

		foot_data[c].prev_frame = display_data.sequence_frame[c];
	}

	return true;

}

bool AnimationControl::updateAnimation(float _elapsed_time)
{
	if (!doneGettingData) {
		return getData(_elapsed_time);
	}
	else {
		return warpTime(_elapsed_time);
	}
}

static Skeleton* buildCharacter(
	Skeleton* _skel,
	MotionSequence* _ms,
	Color _bone_color,
	const string& _description1,
	const string& _description2,
	vector<Object*>& _render_list)
{
	if ((_skel == NULL) || (_ms == NULL)) return NULL;

	OpenMotionSequenceController* controller = new OpenMotionSequenceController(_ms);

	//! Hack. The skeleton expects a list<Object*>, we're using a vector<Object*>
	list<Object*> tmp;
	_skel->constructRenderObject(tmp, _bone_color);
	list<Object*>::iterator iter = tmp.begin();
	while (iter != tmp.end()) { _render_list.push_back(*iter); iter++; }
	//! EndOfHack.

	_skel->attachMotionController(controller);
	_skel->setDescription1(_description1.c_str());
	_skel->setDescription2(_description2.c_str());
	return _skel;
}

void AnimationControl::loadCharacters()
{
	data_manager.addFileSearchPath(AMC_MOTION_FILE_PATH);
	data_manager.addFileSearchPath(BVH_MOTION_FILE_PATH);

	Skeleton* skel = NULL;
	MotionSequence* ms = NULL;
	string descr1, descr2;
	char* filename1 = NULL;
	char* filename2 = NULL;
	Skeleton* character = NULL;
	pair<Skeleton*, MotionSequence*> read_result;

	for (short c = 0; c < NUM_CHARACTERS; c++)
	{
		if (load_specs[c].mocap_type == AMC)
		{
			try
			{
				filename1 = data_manager.findFile(load_specs[c].skeleton_file.c_str());
				if (filename1 == NULL)
				{
					logout << "AnimationControl::loadCharacters: Unable to find character ASF file <" << load_specs[c].skeleton_file << ">. Aborting load." << endl;
					throw BasicException("ABORT 1A");
				}
				filename2 = data_manager.findFile(load_specs[c].motion_file.c_str());
				if (filename2 == NULL)
				{
					logout << "AnimationControl::loadCharacters: Unable to find character AMC file <" << load_specs[c].motion_file << ">. Aborting load." << endl;
					throw BasicException("ABORT 1B");
				}
				try {
					read_result = data_manager.readASFAMC(filename1, filename2);
				}
				catch (const DataManagementException& dme)
				{
					logout << "AnimationControl::loadCharacters: Unable to load character data files. Aborting load." << endl;
					logout << "   Failure due to " << dme.msg << endl;
					throw BasicException("ABORT 1C");
				}
			}
			catch (BasicException&) {}
		}
		else if (load_specs[c].mocap_type == BVH)
		{
			try
			{
				filename1 = data_manager.findFile(load_specs[c].motion_file.c_str());
				if (filename1 == NULL)
				{
					logout << "AnimationControl::loadCharacters: Unable to find character BVH file <" << load_specs[c].motion_file << ">. Aborting load." << endl;
					throw BasicException("ABORT 2A");
				}
				try
				{
					read_result = data_manager.readBVH(filename1);
				}
				catch (const DataManagementException& dme)
				{
					logout << "AnimationControl::loadCharacters: Unable to load character data files. Aborting load." << endl;
					logout << "   Failure due to " << dme.msg << endl;
					throw BasicException("ABORT 2C");
				}
			}
			catch (BasicException&) {}
		}

		try
		{
			skel = read_result.first;
			ms = read_result.second;

			skel->scaleBoneLengths(load_specs[c].scale);
			ms->scaleChannel(CHANNEL_ID(0, CT_TX), load_specs[c].scale);
			ms->scaleChannel(CHANNEL_ID(0, CT_TY), load_specs[c].scale);
			ms->scaleChannel(CHANNEL_ID(0, CT_TZ), load_specs[c].scale);

			// create a character to link all the pieces together.
			descr1 = string("skeleton: ") + load_specs[c].skeleton_file;
			descr2 = string("motion: ") + load_specs[c].motion_file;

			character = buildCharacter(skel, ms, load_specs[c].color, descr1, descr2, render_lists.bones);
			if (character != NULL) characters.push_back(character);
		}
		catch (BasicException&) {}

		strDelete(filename1); filename1 = NULL;
		strDelete(filename2); filename2 = NULL;
	}

	display_data.num_characters = (short)characters.size();
	display_data.sequence_time.resize(characters.size());
	display_data.sequence_frame.resize(characters.size());

	if (characters.size() > 0) ready = true;
}

