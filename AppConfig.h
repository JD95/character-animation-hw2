//-----------------------------------------------------------------------------
// HW02 - Builds with SKA Version 4.0
//-----------------------------------------------------------------------------
// AppConfig.h
//    Define application specific things, such as file locations.
//-----------------------------------------------------------------------------
#ifndef APPCONFIG_DOT_H
#define APPCONFIG_DOT_H
// SKA configuration
#include <Core/SystemConfiguration.h>

// root path to the ASF/AMC files
#define AMC_MOTION_FILE_PATH "../../data/motion/AMC"
// root path to the BVH files
#define BVH_MOTION_FILE_PATH "../../data/motion/BVH"
// textures are BMP files that are used to color some objects (such as the sky)
#define TEXTURE_FILE_PATH "../../data/textures"

#endif // APPCONFIG_DOT_H
