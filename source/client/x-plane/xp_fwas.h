#ifndef XP_FWAS_H
#define XP_FWAS_H

//Include FoxWorks Aerospace Simulator
#include "fwas.h"

//Include X-Plane SDK
#include <XPLMCamera.h>
#include <XPLMDefs.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMGraphics.h>
#include <XPLMUtilities.h>
#include <XPLMPlanes.h>
#include <XPLMPlugin.h>
#include <XPLMProcessing.h>
#include <XPLMScenery.h>
#include <XPLMMenus.h>
#include <XPWidgets.h>
#include <XPStandardWidgets.h>

//Include OpenGL
#if IBM
#	include <windows.h>
#	include <GL/gl.h>
#	include <GL/glu.h>
//#	include <glext.h>
#elif LIN
#	ifndef TRUE
#		define TRUE 1
#		define FALSE 0
#	endif
#	define GL_GLEXT_PROTOTYPES
#	include <GL/gl.h>
#	include <GL/glu.h>
//#	include <glext.h>
#else
#	ifndef TRUE
#		define TRUE 1
#		define FALSE 0
#	endif
#	if APL
#		define GL_GLEXT_PROTOTYPES
#		include <OpenGL/gl.h>
#		include <OpenGL/glu.h>
#		if (GL_GLEXT_VERSION < 44)
#			undef __glext_h_
#		endif
//#		include <glext.h>
#	else
#		define GL_GLEXT_PROTOTYPES
#		include <gl.h>
#		include <glu.h>
//#		include <glext.h>
#	endif
#endif

//Some arbitrary constants
#define MAX_FILENAME		((32760 + 255 + 1)*2)
#define ARBITRARY_MAX		16384
#ifdef _WIN32
#	define snprintf _snprintf
#	define snscanf _snscanf
#endif

// Draw a single mesh
void XPFWAS_DrawMesh(EVDS_MESH* mesh);
// Draw an object
void XPFWAS_DrawObject(EVDS_OBJECT* object);

#endif
