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
/// Thread that generates LOD meshes
////////////////////////////////////////////////////////////////////////////////
SIMC_LOCK_ID FWAS_EVDS_MeshGenerate_ThreadsRunningLock = SIMC_THREAD_BAD_ID;
int FWAS_EVDS_MeshGenerate_ThreadsRunning = 0;
void FWAS_EVDS_MeshGenerate_Thread(FWAS_EVDS_USERDATA* userdata) {
	int lod;

	//Wait until number of thread falls down
	while (1) {
		SIMC_Lock_Enter(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);

		if (FWAS_EVDS_MeshGenerate_ThreadsRunning < SIMC_Thread_GetNumProcessors()) {
			FWAS_EVDS_MeshGenerate_ThreadsRunning++;
			SIMC_Lock_Leave(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
			break;
		}

		SIMC_Lock_Leave(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
		SIMC_Thread_Sleep(0.0f);
	}

	//Generate mesh for the object
	for (lod = 1; lod < FWAS_LOD_LEVELS; lod++) { //Generate LODs from the last
		float resolution = 0.05f * powf(1.5f,FWAS_LOD_LEVELS-lod-1);

		EVDS_Mesh_Generate(userdata->object,&userdata->mesh[lod],resolution,0);
		userdata->lod_count++; //Signal that LOD is present
	}	

	//Remove a thread
	SIMC_Lock_Enter(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
		FWAS_EVDS_MeshGenerate_ThreadsRunning--;
	SIMC_Lock_Leave(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
}


////////////////////////////////////////////////////////////////////////////////
/// Thread that simulates state
////////////////////////////////////////////////////////////////////////////////
void FWAS_EVDS_Simulate_Thread(FWAS* simulator) {
	while (1) {
		if (!simulator->paused) {
			int i;
			EVDS_OBJECT* root;
			EVDS_RigidBody_UpdateDetaching(simulator->system);
			EVDS_System_GetRootInertialSpace(simulator->system,&root);
			for (i = 0; i < 10; i++) {
			EVDS_Object_Solve(root,1.0f/30.0f);
			}
		}
		SIMC_Thread_Sleep(1.0f/30.0f);
	}
}


////////////////////////////////////////////////////////////////////////////////
/// Perform post-initialization for the object
////////////////////////////////////////////////////////////////////////////////
int FWAS_EVDS_Callback_PostInitialize(EVDS_SYSTEM* system, EVDS_SOLVER* solver, EVDS_OBJECT* object) {
	FWAS_EVDS_USERDATA* userdata = malloc(sizeof(FWAS_EVDS_USERDATA));
	memset(userdata,0,sizeof(FWAS_EVDS_USERDATA));

	//Generate initial mesh (zero-level LOD)
	EVDS_Mesh_Generate(object,&userdata->mesh[0],EVDS_MESH_LOWEST_RESOLUTION,0);
	userdata->lod_count = 1;

	//Set as userdata (so renderer can start using the object right away)
	userdata->object = object;
	EVDS_Object_SetUserdata(object,userdata);

	//Start thread to generate meshes
	SIMC_Thread_Create(FWAS_EVDS_MeshGenerate_Thread,userdata);
	return EVDS_OK;
}


////////////////////////////////////////////////////////////////////////////////
/// Initialize simulator
////////////////////////////////////////////////////////////////////////////////
int FWAS_Initialize(FWAS** p_simulator) {
	FWAS* simulator;
	EVDS_GLOBAL_CALLBACKS callbacks = { 0 };

	//Create global locks
	if (!FWAS_EVDS_MeshGenerate_ThreadsRunningLock) {
		FWAS_EVDS_MeshGenerate_ThreadsRunningLock = SIMC_Lock_Create();
	}

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

	//Start simulation thread
	simulator->paused = 1;
	SIMC_Thread_Create(FWAS_EVDS_Simulate_Thread,simulator);
	return 1;
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
"<EVDS version=\"35\">"
"	<object name=\"Earth_Inertial_Space\" type=\"propagator_rk4\">"
"		<object name=\"Earth\" type=\"planet\">"
"			<parameter name=\"gravity.mu\">3.9860044e14</parameter>"		//m3 sec-2
"			<parameter name=\"geometry.radius\">6378.145e3</parameter>"		//m
"			<parameter name=\"information.period\">86164.10</parameter>"	//sec
"			<parameter name=\"is_static\">1</parameter>"
"		</object>"
"	</object>"
"</EVDS>",&solar_system);
	EVDS_Object_Initialize(solar_system,1);
}


////////////////////////////////////////////////////////////////////////////////
/// Load vessel(s) from file
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Vessel_LoadFromFile(FWAS* simulator, EVDS_OBJECT* parent, char* filename) {
	char reference[1025] = { 0 };
	EVDS_OBJECT* vessel;
	EVDS_Object_GetReference(parent,0,reference,1024);
	simulator->log(FWAS_MESSAGE_INFO,"FWAS_Vessel_Load: Load from %s into %s...",filename,reference);

	//Load the vessel
	EVDS_Object_LoadFromFile(parent,filename,&vessel);
	EVDS_Object_Initialize(vessel,0);
	return vessel;
}


////////////////////////////////////////////////////////////////////////////////
/// Iterate through every child of the parent
////////////////////////////////////////////////////////////////////////////////
void FWAS_Vessel_IterateChildren(FWAS* simulator, EVDS_OBJECT* parent, FWAS_Callback_Vessel* callback) {
	SIMC_LIST* list;
	SIMC_LIST_ENTRY* entry;

	EVDS_Object_GetChildren(parent,&list);
	entry = SIMC_List_GetFirst(list);
	while (entry) {
		EVDS_OBJECT* child = SIMC_List_GetData(list,entry);
		callback(child);
		entry = SIMC_List_GetNext(list,entry);
	}
}


////////////////////////////////////////////////////////////////////////////////
/// Set active vessel
////////////////////////////////////////////////////////////////////////////////
void FWAS_Vessel_SetActive(FWAS* simulator, EVDS_OBJECT* vessel) {
	simulator->active_vessel = vessel;	
}


////////////////////////////////////////////////////////////////////////////////
/// Get planet by name
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Planet_GetByName(FWAS* simulator, char* name) {
	EVDS_OBJECT* planet;
	if (EVDS_System_GetObjectByName(simulator->system,name,0,&planet) != EVDS_OK) {
		simulator->log(FWAS_MESSAGE_ERROR,"FWAS_Planet_GetByName: Not found: %s",name);
		return 0;
	} else {
		return planet;
	}
}