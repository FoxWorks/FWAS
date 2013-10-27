////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
/// Copyright (C) 2012-2013, Black Phoenix
///
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the GNU Lesser General Public License as published by the Free Software
/// Foundation; either version 2 of the License, or (at your option) any later
/// version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
/// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
/// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
/// details.
///
/// You should have received a copy of the GNU Lesser General Public License along with
/// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
/// Place - Suite 330, Boston, MA  02111-1307, USA.
///
/// Further information about the GNU Lesser General Public License can also be found on
/// the world wide web at http://www.gnu.org.
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "fwas.h"


////////////////////////////////////////////////////////////////////////////////
/// Initialize object
////////////////////////////////////////////////////////////////////////////////
int FWAS_EVDS_Callback_PostInitialize(EVDS_SYSTEM* system, EVDS_SOLVER* solver, EVDS_OBJECT* object) {
	FWAS_EVDS_USERDATA* userdata = malloc(sizeof(FWAS_EVDS_USERDATA));
	memset(userdata,0,sizeof(FWAS_EVDS_USERDATA));

	//Generate mesh for the object
	userdata->object = object;
	EVDS_Mesh_Generate(object,&userdata->mesh,0.05f,0);//025

	//Set as userdata
	EVDS_Object_SetUserdata(object,userdata);
	return EVDS_OK;
}


////////////////////////////////////////////////////////////////////////////////
/// Initialize simulator
////////////////////////////////////////////////////////////////////////////////
int FWAS_Initialize(FWAS** p_simulator) {
	FWAS* simulator;
	EVDS_GLOBAL_CALLBACKS callbacks = { 0 };

	//Create new simulator
	simulator = malloc(sizeof(FWAS));
	*p_simulator = simulator;
	if (!simulator) return 0;
	memset(simulator,0,sizeof(FWAS));

	//Initialize EVDS
	EVDS_System_Create(&simulator->system);
	EVDS_Common_Register(simulator->system);
	EVDS_Train_WheelsGeometry_Register(simulator->system);

	//Set proper callbacks
	callbacks.OnInitialize = 0;
	callbacks.OnPostInitialize = FWAS_EVDS_Callback_PostInitialize;
	callbacks.OnDeinitialize = 0;
	EVDS_System_SetGlobalCallbacks(simulator->system,&callbacks);
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// Set logging callback
////////////////////////////////////////////////////////////////////////////////
void FWAS_SetCallback_Log(FWAS* simulator, FWAS_Callback_Log* onLog) {
	simulator->log = onLog;
}


////////////////////////////////////////////////////////////////////////////////
/// Deinitialize simulator
////////////////////////////////////////////////////////////////////////////////
void FWAS_Deinitialize(FWAS* simulator) {

}


////////////////////////////////////////////////////////////////////////////////
/// Load Earth and Moon simulation scene
////////////////////////////////////////////////////////////////////////////////
void FWAS_LoadScene_EarthMoon(FWAS* simulator) {
	EVDS_OBJECT* root;
	EVDS_OBJECT* solar_system;
	simulator->log(FWAS_MESSAGE_INFO,"FWAS_SetScene_EarthMoon: loading default scene");

	EVDS_System_GetRootInertialSpace(simulator->system,&root);
	EVDS_Object_LoadFromString(root,
"		<object name=\"Earth_Inertial_Space\" type=\"propagator_rk4\">"
"			<object name=\"Earth\" type=\"planet\">"
"				<parameter name=\"gravity.mu\">3.9860044e14</parameter>"		//m3 sec-2
"				<parameter name=\"geometry.radius\">6378.145e3</parameter>"		//m
"				<parameter name=\"information.period\">86164.10</parameter>"	//sec
"				<parameter name=\"is_static\">1</parameter>"
"			</object>"
"		</object>",&solar_system);

	//Create moon
	/*EVDS_Object_Create(inertial_earth,&planet_earth_moon);
	EVDS_Object_SetType(planet_earth_moon,"planet");
	EVDS_Object_SetName(planet_earth_moon,"Moon");
	EVDS_Object_AddRealVariable(planet_earth_moon,"mu",0.0490277e14,0);	//m3 sec-2
	EVDS_Object_AddRealVariable(planet_earth_moon,"radius",1737e3,0);		//m
	EVDS_Object_SetPosition(planet_earth_moon,inertial_earth,0.0,362570e3,0.0);
	EVDS_Object_SetVelocity(planet_earth_moon,inertial_earth,1000.0,0.0,0.0);
	EVDS_Object_Initialize(planet_earth_moon,1);*/
}


////////////////////////////////////////////////////////////////////////////////
/// Load a vessel from file
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Vessel_LoadFromFile(FWAS* simulator, char* filename) {
	EVDS_OBJECT *root,*vessel;
	simulator->log(FWAS_MESSAGE_INFO,"FWAS_Vessel_Load: %s",filename);

	//Load the vessel
	EVDS_System_GetRootInertialSpace(simulator->system,&root);	
	EVDS_Object_LoadFromFile(root,filename,&vessel);
	EVDS_Object_Initialize(vessel,1);
	return vessel;
}


////////////////////////////////////////////////////////////////////////////////
/// Load a vessel from file and place it somewhere on the planet
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Vessel_LoadAndPlace(FWAS* simulator, EVDS_GEODETIC_COORDINATE* location, char* filename) {
	EVDS_OBJECT* vessel;
	EVDS_STATE_VECTOR vector;
	simulator->log(FWAS_MESSAGE_INFO,"FWAS_Vessel_LoadAndPlace: %s",filename);

	//Load the vessel
	EVDS_Object_LoadFromFile(location->datum.object,filename,&vessel);

	//Place vessel at location and initialize it
	EVDS_Object_GetStateVector(vessel,&vector);
	EVDS_Geodetic_ToVector(&vector.position,location);
	EVDS_Object_SetStateVector(vessel,&vector);
	EVDS_Object_Initialize(vessel,1);
	return vessel;
}


////////////////////////////////////////////////////////////////////////////////
/// Set active vessel
////////////////////////////////////////////////////////////////////////////////
void FWAS_Vessel_SetActive(FWAS* simulator, EVDS_OBJECT* vessel) {
	simulator->active_vessel = vessel;	
}