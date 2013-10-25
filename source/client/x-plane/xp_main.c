#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
/*
#if IBM
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <glext.h>
#elif LIN
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <glext.h>
#else
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#if APL
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <gl.h>
#include <glu.h>
#include <glext.h>
#endif
#endif*/

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

#define MAX_FILENAME		((32760 + 255 + 1)*2)
#define ARBITRARY_MAX		16384
#ifdef _WIN32
#define snprintf _snprintf
#define snscanf _snscanf
#endif


//==============================================================================
// Log something to X-Plane's Log.txt
//==============================================================================
void log_write(char* text, ...)
{
	char buf[ARBITRARY_MAX] = { 0 };
	va_list args;

	va_start(args, text);
	vsnprintf(buf,ARBITRARY_MAX-1,text,args);
	XPLMDebugString(buf);
	va_end(args);
}




//==============================================================================
// X-Plane callback for drawing
//==============================================================================
int XPluginDrawCallback(XPLMDrawingPhase phase, int isBefore, void* refcon)
{
	/*int i;
	if (phase == xplm_Phase_LastCockpit) {
		if (ivss_system) {
			if (!ivss_video_processor_initialized) {
				ivss_video_processor_initialized = 1;
				for (i = 0; i < video_processors->count; i++) {
					IVSS_Simulator_XGDC_VideoProcessor_Startup(ivss_system,video_processors->units[i]);
				}
			}

			for (i = 0; i < video_processors->count; i++) {
				IVSS_Simulator_XGDC_VideoProcessor_Frame(ivss_system,video_processors->units[i]);
			}
		}
	} else {
		int i,w,h;
		XPLMGetScreenSize(&w,&h);

		for (i = 0; i < 80; i++) {
			float RGB[4] = { 1.0f,1.0f,1.0f,1.0f };
			if (debug_buf[i]) XPLMDrawString(RGB,12*2,12*(i+2),debug_buf[i],0,xplmFont_Basic);
		}
	}*/
	return 1;
}


//==============================================================================
// X-Plane callback for flight loop
//==============================================================================
float XPluginFlightLoop(float elapsedSinceLastCall, float elapsedTimeSinceLastFlightLoop,
                        int counter, void* refcon)
{
	/*int i;
	if (ivss_system && (counter > 0)) {
		ivss_system->is_paused = XPLMGetDatai(simulator_paused);

		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// This is the interesting part. This "simulates" variables from inside X-Plane to make them work with IVSS
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		for (i = 0; i < xplane_datarefs->count; i++) {
			IVSS_Interface_XPlane_Frame(ivss_system,xplane_datarefs->units[i]);
		}

		if (!ivss_sensor_processor_initialized) {
			ivss_sensor_processor_initialized = 1;
			for (i = 0; i < sensor_processors->count; i++) {
				IVSS_Simulator_XGDC_MDMProcessor_Startup(ivss_system,sensor_processors->units[i]);
			}
		}

		for (i = 0; i < sensor_processors->count; i++) {
			IVSS_Simulator_XGDC_MDMProcessor_Frame(ivss_system,sensor_processors->units[i]);
		}
	}*/
	return -1;
}


//==============================================================================
// Reinitialize IVSS
//==============================================================================
void xivss_reinitialize() {
	/*char path[MAX_FILENAME] = { 0 }, model[MAX_FILENAME] = { 0 };
	char buf[ARBITRARY_MAX] = { 0 };
	char* acfpath;
	extern char avcl_translate_prefix[8192];

	//Deinitialize IVSS system
	if (ivss_system) IVSS_System_Destroy(&ivss_system);
	ivss_video_processor_initialized = 0;
	ivss_sensor_processor_initialized = 0;

	//Read aircraft path and path separator
	XPLMGetNthAircraftModel(0, model, path);
	XPLMExtractFileAndPath(path);

	//If not yet loading anything, skip
	if (!model[0]) return;

	//Fetch only aircraft folder and replace paths under Mac OS
	acfpath = strstr(path,"Aircraft");
	if (acfpath) snprintf(path,MAX_FILENAME-1,".\\%s",acfpath);
#ifdef APL
	while (acfpath = strchr(path,':')) *acfpath = '/';
#endif

	//Fetch only aircraft model name
	acfpath = strstr(model,".acf");
	if (acfpath) *acfpath = 0;

	//Generate name for the IVSS system description
	strncpy(buf,path,ARBITRARY_MAX-1);
	strncat(buf,"\\systems.ivss",ARBITRARY_MAX-1);

	//Generate prefix for AVCL file operations (small hack)
	strncpy(avcl_translate_prefix,path,8191);
	strncat(avcl_translate_prefix,"\\",8191);

	//Try to initialize system
	IVSS_System_Create(&ivss_system);
	IVSS_Simulator_XGDC_Register(ivss_system);
	IVSS_Simulator_DSP_Register(ivss_system);
	IVSS_Interface_XPlane_Register(ivss_system);
	log_write("X-IVSS: Loading internal systems simulation description for %s\n",model);
	if (IVSS_System_LoadFromFile(ivss_system,buf)) {
		log_write("X-IVSS: Loading error: %s\n",IVSS_System_GetError(ivss_system));
		IVSS_System_Destroy(&ivss_system);
	} else {
		ivss_system->is_paused = 1;
		IVSS_System_Reset(ivss_system);
		IVSS_System_GetUnitsByClassType(ivss_system,"XGDC",IVSS_TYPE_XGDC_VIDEO_PROCESSOR,&video_processors);
		IVSS_System_GetUnitsByClassType(ivss_system,"XGDC",IVSS_TYPE_XGDC_MDM_PROCESSOR,&sensor_processors);
		IVSS_System_GetUnitsByClassType(ivss_system,"X-Plane",IVSS_TYPE_XPLANE_DATAREF,&xplane_datarefs);
	}*/
}


//==============================================================================
// Called on plugin startup
//==============================================================================
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc)
{
	strcpy(outName, "FWAS X-Plane Client");
	strcpy(outDesc, "X-Plane client for FoxWorks Aerospace Simulator");
	strcpy(outSig, "xsag.fwas_x-plane");

	//video_scroll_x = XPLMFindDataRef("sim/graphics/misc/current_scroll_pos_x");
	//video_scroll_y = XPLMFindDataRef("sim/graphics/misc/current_scroll_pos");
	//simulator_paused = XPLMFindDataRef("sim/time/paused");

	XPLMDebugString("FWAS: Client initialized\n");
	return 1;
}


/*******************************************************************************
 * Plugin initialization
 ******************************************************************************/
PLUGIN_API int XPluginEnable(void)
{
	xivss_reinitialize();
	log_write("FWAS: Client started\n");

	//Register callbacks
	XPLMRegisterFlightLoopCallback(XPluginFlightLoop, -1, NULL);
	//XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Terrain, 0, NULL);
	//XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes, 0, NULL);
	//XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes, 1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Window, 0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_LastCockpit, 0, NULL);
	return 1;
}


/*******************************************************************************
 * Plugin deinitialization
 ******************************************************************************/
PLUGIN_API void XPluginDisable(void)
{
	//if (ivss_system) IVSS_System_Destroy(&ivss_system);
	log_write("FWAS: Client stopped\n");

	//Unregister callbacks
	XPLMUnregisterFlightLoopCallback(XPluginFlightLoop, NULL);
	//XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_FirstScene, 0, NULL);
	//XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes, 0, NULL);
	//XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes, 1, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Window, 0, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_LastCockpit, 0, NULL);
}


/*******************************************************************************
 * Message handler
 ******************************************************************************/
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID fromWho, long message, void* param)
{
	if ((message == XPLM_MSG_PLANE_LOADED) && (!param)) {
		xivss_reinitialize();
	}
}