////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "xp_fwas.h"
#ifdef _WIN32
#	include "windows.h"
#endif




////////////////////////////////////////////////////////////////////////////////
/// Local state of the currently active vessel (overrides X-Plane camera)
////////////////////////////////////////////////////////////////////////////////
FWAS* fwas;
EVDS_OBJECT* homebase;
EVDS_OBJECT* earth;
EVDS_OBJECT* earth_inertial_space;

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
/// FWAS logging callback (writes to X-Plane log.txt)
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_Log(int level, char* message, ...) {
	char buf[ARBITRARY_MAX] = { 0 };
	va_list args;

	va_start(args, message);
	vsnprintf(buf,ARBITRARY_MAX-1,message,args);
	XPLMDebugString(buf);
	va_end(args);
#ifdef _WIN32
	strncat(buf,"\n",ARBITRARY_MAX);
	OutputDebugString(buf);
#endif
}




////////////////////////////////////////////////////////////////////////////////
/// X-Plane callback for drawing
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_Callback_DrawVessel(EVDS_OBJECT* object) {
	char name[256] = { 0 };
	EVDS_Object_GetName(object,name,255);
	XPFWAS_DrawObject(object);
}

int XPluginDrawCallback(XPLMDrawingPhase phase, int isBefore, void* refcon) {
	float dX  = 20.0f;
	float dYZ = 20.0f;

	switch (phase) {
		case xplm_Phase_Airplanes:
			XPLMSetGraphicsState(0,0,1,0,0,1,1);
			FWAS_Vessel_IterateChildren(fwas,earth,XPFWAS_Callback_DrawVessel);
			FWAS_Vessel_IterateChildren(fwas,earth_inertial_space,XPFWAS_Callback_DrawVessel);
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
	EVDS_QUATERNION quaternion;
	double x,y,z;

	EVDS_GEODETIC_COORDINATE geocoord;
	EVDS_GEODETIC_DATUM datum;
	EVDS_STATE_VECTOR vector;
	EVDS_OBJECT* vessel = homebase;
	if (fwas->active_vessel) {
		vessel = fwas->active_vessel;
	}

	//Get location of the vessel in geographic coordinates
	EVDS_Object_GetStateVector(vessel,&vector);
	EVDS_Geodetic_DatumFromObject(&datum,earth);
	EVDS_Geodetic_FromVector(&geocoord,&vector.position,&datum);

	//Mimic state of the active vessel for X-Plane camera
	XPLMWorldToLocal(geocoord.latitude,geocoord.longitude,geocoord.elevation,&x,&y,&z);
	EVDS_LVLH_QuaternionToLVLH(&quaternion,&vector.orientation,&geocoord);
	//q[0] = (float)quaternion.q[0];
	//q[1] = (float)quaternion.q[1];
	//q[2] = (float)quaternion.q[2];
	//q[3] = (float)quaternion.q[3];

	XPLMSetDatad(dataref_x,x);
	XPLMSetDatad(dataref_y,y);
	XPLMSetDatad(dataref_z,z);
	XPLMSetDataf(dataref_vx,0.0);
	XPLMSetDataf(dataref_vy,0.0);
	XPLMSetDataf(dataref_vz,0.0);
	XPLMSetDataf(dataref_P,0.0);
	XPLMSetDataf(dataref_Q,0.0);
	XPLMSetDataf(dataref_R,0.0);
	XPLMSetDatavf(dataref_q,q,0,4);

	//Synchronize simulation with X-Plane
	fwas->paused = XPLMGetDatai(dataref_paused);

	//Do some test
	{
		EVDS_OBJECT* gimbal;
		if (EVDS_System_GetObjectByName(fwas->system,"RD-171",0,&gimbal) == EVDS_OK) {
			EVDS_VARIABLE* pitch_command;
			EVDS_VARIABLE* yaw_command;
			EVDS_Object_GetVariable(gimbal,"pitch.command",&pitch_command);
			EVDS_Object_GetVariable(gimbal,"yaw.command",&yaw_command);

			if ((counter % 100) > 50) {
				EVDS_Variable_SetReal(pitch_command,10.0);
			} else {
				EVDS_Variable_SetReal(pitch_command,-10.0);
			}

			if ((counter % 200) > 100) {
				EVDS_Variable_SetReal(yaw_command,10.0);
			} else {
				EVDS_Variable_SetReal(yaw_command,-10.0);
			}
		}
	}

	if (fwas->paused) {
		EVDS_VARIABLE* variable;
		if (EVDS_Object_GetVariable(fwas->active_vessel,"detach",&variable) == EVDS_OK) {
			EVDS_Variable_SetReal(variable,1.0);
		}
	}
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
	XPFWAS_Log(FWAS_MESSAGE_INFO,"FWAS-XP: Client initialized\n");
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// Plugin initialization
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API int XPluginEnable(void) {
	//Initialize simulator
	FWAS_Initialize(&fwas);

	//Setup callbacks
	fwas->log = XPFWAS_Log;
	XPFWAS_Log(FWAS_MESSAGE_INFO,"FWAS-XP: Client started\n");

	//Set scene matching X-Plane world
	FWAS_LoadScene_EarthMoon(fwas);
	earth = FWAS_Planet_GetByName(fwas,"Earth");
	earth_inertial_space = FWAS_Planet_GetByName(fwas,"Earth_Inertial_Space");

	//Add home base
	homebase = FWAS_Vessel_LoadFromFile(fwas,earth,"./Resources/plugins/fwas_x-plane/xsag_launchpad.evds");
	FWAS_Vessel_SetActive(fwas,FWAS_Vessel_LoadFromFile(fwas,homebase,"./Resources/plugins/fwas_x-plane/rv505.evds"));

	//Register callbacks
	XPLMRegisterFlightLoopCallback(XPluginFlightLoop, -1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,		0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Airplanes,		1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,			0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Panel,			1, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		0, NULL);
	XPLMRegisterDrawCallback(XPluginDrawCallback, xplm_Phase_Gauges,		1, NULL);

	//Disable motion of players plane
	XPLMDisableAIForPlane(0);
	XPLMSetActiveAircraftCount(1);
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// Plugin deinitialization
////////////////////////////////////////////////////////////////////////////////
PLUGIN_API void XPluginDisable(void) {
	FWAS_Deinitialize(fwas);
	XPFWAS_Log(FWAS_MESSAGE_INFO,"FWAS-XP: Client stopped\n");

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