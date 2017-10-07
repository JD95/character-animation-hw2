//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// AppMain.cpp
//    The main program is mostly the connection between openGL,
//    SKA and application specific code. It also controls the order of
//    initialization, before control is turned over to openGL.
//-----------------------------------------------------------------------------
// SKA configuration.
#include <Core/SystemConfiguration.h>
// C/C++ libraries
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;
// openGL library
#include <GL/glut.h>
// SKA modules
#include <Core/BasicException.h>
#include <Core/SystemTimer.h>
#include <Core/Utilities.h>
#include <DataManagement/DataManagementException.h>
#include <Input/InputManager.h>
#include <Graphics/GraphicsInterface.h>
#include <Graphics/Lights.h>
#include <Graphics/Textures.h>
#include <Graphics/Graphics2D.h>
#include <Models/SphereModels.h>
// local application
#include "AppConfig.h"
#include "AnimationControl.h"
#include "CameraControl.h"
#include "InputProcessing.h"
#include "RenderLists.h"

// default window size
static int window_height = 800;
static int window_width = 800;

// which objects background objects do we want to see?
static bool SHOW_SKY = false;
static bool SHOW_GROUND = true;
static bool SHOW_COORD_AXIS = false;

//  background color (black)
static float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f};

void shutDown(int _exit_code)
{
	exit(_exit_code);
}

// heads-up display = 2D text on screen
void drawHUD()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	Color color(1.0f,1.0f,0.0f,1.0f);
	// five columns
	float x1 = -0.8f, x2 = -0.4f, x3 = 0.0f, x4 = 0.25f, x5 = 0.5f;
	// row cursor
	float y;
	float row_height = 0.05f;

	string s;

	// first column
	y = 0.9f;
	s = "Global Time: ";
	renderString(x1, y, 0.0f, color, s.c_str());
	s = toString(anim_ctrl.getRunTime());
	renderString(x2, y, 0.0f, color, s.c_str());
	
	y -= row_height;

	s = "Global Time Warp: ";
	renderString(x1, y, 0.0f, color, s.c_str());
	s = toString(anim_ctrl.getGlobalTimeWarp());
	renderString(x2, y, 0.0f, color, s.c_str());

	y -= row_height;

	y = 0.9f;
	s = "Character: ";
	renderString(x3, y, 0.0f, color, s.c_str());
	s = "Time: ";
	renderString(x4, y, 0.0f, color, s.c_str());
	s = "Frame: ";
	renderString(x5, y, 0.0f, color, s.c_str());

	y -= row_height;

	for (short i = 0; i < display_data.num_characters; i++)
	{
		s = toString(i);
		renderString(x3, y, 0.0f, color, s.c_str());
		s = toString(display_data.sequence_time[i]);
		renderString(x4, y, 0.0f, color, s.c_str());
		s = toString(display_data.sequence_frame[i]);
		renderString(x5, y, 0.0f, color, s.c_str());
		y -= row_height;
	}
}

// display() is the call back function from the openGL rendering loop.
// All recurring processing is initiated from this function.
void display(void)
{
	// Determine how much time has passed since the previous frame.
	double elapsed_time = system_timer.elapsedTime();

	// Check to see if any user inputs have been received since the last frame.
	input_processor.processInputs(elapsed_time);

	// Set up openGL to draw next frame.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	camera.setSceneView();
	glMatrixMode(GL_MODELVIEW);

	// draw background objects
	for (unsigned short b=0; b < render_lists.background.size(); b++)
	{
		Object* go = render_lists.background[b];
		if (go->isVisible())
		{
			Matrix4x4 world_xform;
			go->render(world_xform);
		}
	}

	// draw erasable objects
	for (unsigned short b = 0; b < render_lists.erasables.size(); b++)
	{
		Object* go = render_lists.erasables[b];
		if (go->isVisible())
		{
			Matrix4x4 world_xform;
			go->render(world_xform);
		}
	}

	// Tell the animation subsystem to update the character, then redraw it.
	if (anim_ctrl.isReady())
	{
		if (anim_ctrl.updateAnimation(elapsed_time))
		{
			for (unsigned short b = 0; b < render_lists.bones.size(); b++)
			{
				Object* go = render_lists.bones[b];
				if (go->isVisible())
				{
					Matrix4x4 world_xform;
					go->render(world_xform);
				}
			}
		}
	}

	// draw the heads-up display
	drawHUD();

	// Activate the new frame.
	glutSwapBuffers();

	// Record any redering errors.
	checkOpenGLError(203);
}

void buildObjects()
{
	// tell texture manager where to find texture BMP files
	texture_manager.addTextureFilepath((char*)TEXTURE_FILE_PATH);

	if (SHOW_SKY)
	{
		// create a sky model directly by creating an instance of InvertedSphereModel
		// which is a textured model
		SphereModel* skymod = new InvertedSphereModel(800, 3,
			Color(1.0f,1.0f,0.5f),(char*)"skymap1.bmp");
		// build a sky object associated with the sky model
		Object* sky = new Object(skymod, Vector3D(0.0f,0.0f,0.0f), Vector3D(0.0f,0.0f,0.0f));
		// store sky object for rendering
		render_lists.background.push_back(sky);
	}

	if (SHOW_GROUND)
	{
		// create a ground model indirectly by defining a ModelSpecification
		ModelSpecification groundspec("Ground");
		// build a ground object associated with the ground model
		Object* ground = new Object(groundspec,
			Vector3D(0.0f,0.0f,0.0f), Vector3D(0.0f,0.0f,0.0f), Vector3D(1.0f,1.0f,1.0f));
		// store ground object for rendering
		render_lists.background.push_back(ground);
	}

	if (SHOW_COORD_AXIS)
	{
		// create a coordinate axes model indirectly by defining a ModelSpecification
		ModelSpecification caxisspec("CoordinateAxis");
		caxisspec.addSpec("length", "100");
		// build a coordinate axes object associated with the coordinate axes model
		Object* caxis = new Object(caxisspec,
			Vector3D(0.0f,0.0f,0.0f), Vector3D(0.0f,0.0f,0.0f));
		// store coordinate axes object for rendering
		render_lists.background.push_back(caxis);
	}
}

// reshape() is a call back from openGL to indicate that the window has
// changed size. Adjust the camera and view parameters accordingly.
static void reshape(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	window_width = w; window_height = h;
	glViewport(0, 0, window_width, window_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	camera.projectionParameters(
		40.0,									// vertical FOV
		float(window_width)/window_height,		// aspect ratio
		1.0,									// near plane
		1000.0);								// far plane
}

// initializeRenderer() is called one time during initialization.
// It sets various openGL parameters that control rendering.
void initializeRenderer()
{
    glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glShadeModel(GL_SMOOTH);
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// initializeGLUT() is called one time during initialization.
// It initializes GLUT.
void initializeGLUT(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_width, window_height);
	glutCreateWindow("HW02 Animation");
}

int main(int argc, char **argv)
{
	// initialize the animation subsystem, which reads the
	// mocap data files and sets up the character(s)
	anim_ctrl.loadCharacters();
	if (!anim_ctrl.isReady())
	{
		logout << "main(): Unable to load characters. Aborting program." << endl;
		return 1;
	}

	// initialize openGL and enter its rendering loop.
	try
	{
		initializeGLUT(argc, argv);
		initializeRenderer();
		// The input manager needs to have register call back function with GLUT
		// to receive keyboard and mouse actions. This needs to happen after GLUT
		// initialization, so it is implemented in a member functions, rather than
		// the InputManager constructor.
		input_manager.registerGlutHandlers();

		glutReshapeFunc(reshape);
		glutDisplayFunc(display);
		glutIdleFunc(display);

		// Call into the SKA Graphics module to select the default lights.
		initializeDefaultLighting();

		// Application specific initialization of the camera/viewpoint.
		camera.initializeCamera(window_width, window_height);

		// construct background objects
		buildObjects();

		// Record any openGL errors.
		checkOpenGLError(202);

		// Start the global system timer/clock.
		system_timer.reset();

		// Jump into the openGL render loop.
		glutMainLoop();
	}
	catch (BasicException& excpt)
	{
		logout << "BasicException caught at top level." << endl;
		logout << "Exception message: " << excpt.msg << endl;
		logout << "Aborting program." << endl;
		cerr << "Aborting due to exception. See log file for details." << endl;
		exit(1);
	}
}
