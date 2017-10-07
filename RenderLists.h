//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// RenderLists.h
//    Lists of objects shared between the animation controller and 
//    the graphics renderer. All entries in these lists should be
//    pointers to renderable objects.
//-----------------------------------------------------------------------------
#ifndef RENDERLISTS_DOT_H
#define RENDERLISTS_DOT_H
// SKA configuration
#include <Core/SystemConfiguration.h>
// C/C++ libraries
#include <list>
#include <vector>
using namespace std;
// SKA modules
#include <Objects/Object.h>

struct RenderLists {
	vector<Object*> bones;
	vector<Object*> background;
	vector<Object*> erasables;

	void eraseErasables() {
		for (unsigned short i = 0; i < erasables.size(); i++) delete erasables[i];
		erasables.clear();
	}

	void eraseAll() {
		for (unsigned short i = 0; i < bones.size(); i++) delete bones[i];
		bones.clear();
		for (unsigned short i = 0; i < background.size(); i++) delete background[i];
		background.clear();
		for (unsigned short i = 0; i < erasables.size(); i++) delete erasables[i];
		erasables.clear();
	}

	RenderLists() { bones.clear(); background.clear(); erasables.clear(); }
	~RenderLists() { eraseAll(); }
};

struct DisplayData {
	short num_characters;
	vector<float> sequence_time;
	vector<long> sequence_frame;
	void clear() { sequence_time.clear(); sequence_frame.clear(); num_characters = 0; }
};

extern RenderLists render_lists;
extern DisplayData display_data;

#endif // RENDERLISTS_DOT_H
