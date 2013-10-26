////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "xp_fwas.h"




////////////////////////////////////////////////////////////////////////////////
/// Local state of the currently active vessel (overrides X-Plane camera)
////////////////////////////////////////////////////////////////////////////////
FWAS* simulator;
XPLMDataRef dataref_paused;
XPLMDataRef dataref_x;
XPLMDataRef dataref_y;
XPLMDataRef dataref_z;
XPLMDataRef dataref_vx;
XPLMDataRef dataref_vy;
XPLMDataRef dataref_vz;
XPLMDataRef dataref_P;
XPLMDataRef dataref_Q;
XPLMDataRef dataref_R;
XPLMDataRef dataref_q;


////////////////////////////////////////////////////////////////////////////////
/// Log something to X-Plane's Log.txt
////////////////////////////////////////////////////////////////////////////////
void log_write(char* text, ...) {
	char buf[ARBITRARY_MAX] = { 0 };
	va_list args;

	va_start(args, text);
	vsnprintf(buf,ARBITRARY_MAX-1,text,args);
	XPLMDebugString(buf);
	va_end(args);
}




////////////////////////////////////////////////////////////////////////////////
/// X-Plane callback for drawing
////////////////////////////////////////////////////////////////////////////////
int XPluginDrawCallback(XPLMDrawingPhase phase, int isBefore, void* refcon) {
	float dX  = 20.0f;
	float dYZ = 20.0f;

	switch (phase) {
		case xplm_Phase_Airplanes:
			XPLMSetGraphicsState(0,0,1,0,0,1,1);
			glPushMatrix();
			glTranslatef(XPLMGetDataf(dataref_x),XPLMGetDataf(dataref_y),XPLMGetDataf(dataref_z));
			XPFWAS_DrawObject(simulator->test);
			/*glBegin(GL_QUADS);
				glColor3f(0.0f,1.0f,0.0f);
				glVertex3f( dX  , dYZ ,-dYZ );
				glVertex3f(-0.0f, dYZ ,-dYZ );
				glVertex3f(-0.0f, dYZ , dYZ );
				glVertex3f( dX  , dYZ , dYZ );
				glColor3f(1.0f,0.5f,0.0f);
				glVertex3f( dX  ,-dYZ , dYZ );
				glVertex3f(-0.0f,-dYZ , dYZ );
				glVertex3f(-0.0f,-dYZ ,-dYZ );
				glVertex3f( dX  ,-dYZ ,-dYZ );
				glColor3f(1.0f,0.0f,0.0f);
				glVertex3f( dX  , dYZ , dYZ );
				glVertex3f(-0.0f, dYZ , dYZ );
				glVertex3f(-0.0f,-dYZ , dYZ );
				glVertex3f( dX  ,-dYZ , dYZ );
				glColor3f(1.0f,1.0f,0.0f);
				glVertex3f( dX  ,-dYZ ,-dYZ );
				glVertex3f(-0.0f,-dYZ ,-dYZ );
				glVertex3f(-0.0f, dYZ ,-dYZ );
				glVertex3f( dX  , dYZ ,-dYZ );
				glColor3f(0.0f,0.0f,1.0f);
				glVertex3f(-0.0f, dYZ , dYZ );
				glVertex3f(-0.0f, dYZ ,-dYZ );
				glVertex3f(-0.0f,-dYZ ,-dYZ );
				glVertex3f(-0.0f,-dYZ , dYZ );
				glColor3f(1.0f,0.0f,1.0f);
				glVertex3f( dX  , dYZ ,-dYZ );
				glVertex3f( dX  , dYZ , dYZ );
				glVertex3f( dX  ,-dYZ , dYZ );
				glVertex3f( dX  ,-dYZ ,-dYZ );
			glEnd();*/
			glPopMatrix();

		case xplm_Phase_Panel:
		case xplm_Phase_Gauges:
			return 0; //Override X-Plane aircraft rendering
		default:
			return 1;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// X-Plane callback for flight loop
////////////////////////////////////////////////////////////////////////////////
float XPluginFlightLoop(float elapsedSinceLastCall, float elapsedTimeSinceLastFlightLoop,
						int counter, void* refcon) {
	float q[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	XPLMSetDataf(dataref_x,0.0);
	XPLMSetDataf(dataref_y,105.0);
	XPLMSetDataf(dataref_z,0.0);
	XPLMSetDataf(dataref_vx,0.0);
	XPLMSetDataf(dataref_vy,0.0);
	XPLMSetDataf(dataref_vz,0.0);
	XPLMSetDataf(dataref_P,0.0);
	XPLMSetDataf(dataref_Q,0.0);
	XPLMSetDataf(dataref_R,0.0);
	XPLMSetDatavf(dataref_q,q,0,4);
	return -1;
}


////////////////////////////////////////////////////////////////////////////////
/// Called on plugin startup
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
	strcpy(outName, "FWAS X-Plane Client");
	strcpy(outDesc, "X-Plane client for FoxWorks Aerospace Simulator");
	strcpy(outSig, "xsag.fwas_x-plane");

	//Load up datarefs
	dataref_paused = XPLMFindDataRef("sim/time/paused");
	dataref_x  = XPLMFindDataRef("sim/flightmodel/position/local_x");
	dataref_y  = XPLMFindDataRef("sim/flightmodel/position/local_y");
	dataref_z  = XPLMFindDataRef("sim/flightmodel/position/local_z");
	dataref_vx = XPLMFindDataRef("sim/flightmodel/position/local_vx");
	dataref_vy = XPLMFindDataRef("sim/flightmodel/position/local_vy");
	dataref_vz = XPLMFindDataRef("sim/flightmodel/position/local_vz");
	dataref_P  = XPLMFindDataRef("sim/flightmodel/position/Qrad");
	dataref_Q  = XPLMFindDataRef("sim/flightmodel/position/Rrad");
	dataref_R  = XPLMFindDataRef("sim/flightmodel/position/Prad");
	dataref_q  = XPLMFindDataRef("sim/flightmodel/position/q");

	//Finish initializing
	XPLMDebugString("FWAS: Client initialized\n");
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// Plugin initialization
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API int XPluginEnable(void) {
	FWAS_Initialize(&simulator);
	log_write("FWAS: Client started\n");

	//Register callbacks
	XPLMRegisterFlightLoopCallback(XPluginFlightLoop, -1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,		0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,		1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,			0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,			1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		1, NULL);
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// Plugin deinitialization
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API void XPluginDisable(void) {
	FWAS_Deinitialize(simulator);
	log_write("FWAS: Client stopped\n");

	//Unregister callbacks
	XPLMUnregisterFlightLoopCallback(XPluginFlightLoop, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,	0, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,	1, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,		0, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,		1, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		0, NULL);
	XPLMUnregisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		1, NULL);
}


////////////////////////////////////////////////////////////////////////////////
/// Message handler
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID fromWho, long message, void* param)
{
	if ((message == XPLM_MSG_PLANE_LOADED) && (!param)) {
		///....
	}
}