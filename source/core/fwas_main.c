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
/// @brief Thread that generates LOD meshes
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
		float resolution = 0.05f*powf(2.0f,FWAS_LOD_LEVELS-lod-1);

		EVDS_Mesh_Generate(userdata->object,&userdata->mesh[lod],resolution,0);
		userdata->lod_count++; //Signal that LOD is present
	}	

	//Remove a thread
	SIMC_Lock_Enter(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
		FWAS_EVDS_MeshGenerate_ThreadsRunning--;
	SIMC_Lock_Leave(FWAS_EVDS_MeshGenerate_ThreadsRunningLock);
}




////////////////////////////////////////////////////////////////////////////////
/// @brief Thread that simulates state
////////////////////////////////////////////////////////////////////////////////
void FWAS_EVDS_Simulate_Thread(FWAS* simulator) {
	while (1) {
		if (!simulator->paused) {
			EVDS_OBJECT* root;
			EVDS_RigidBody_UpdateDetaching(simulator->system);
			EVDS_System_GetRootInertialSpace(simulator->system,&root);
			EVDS_Object_Solve(root,1.0f/30.0f);
		}
		SIMC_Thread_Sleep(1.0f/30.0f);
	}
}




////////////////////////////////////////////////////////////////////////////////
/// @brief Perform pre-initialization for the object
////////////////////////////////////////////////////////////////////////////////
int FWAS_EVDS_Callback_PreInitialize(EVDS_SYSTEM* system, EVDS_SOLVER* solver, EVDS_OBJECT* object) {
	EVDS_VARIABLE* variable;

	//Add corresponding variables
	EVDS_Object_AddVariable(object,"fwas.stored.state_vector",EVDS_VARIABLE_TYPE_DATA_PTR,&variable);
	return EVDS_OK;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Perform post-initialization for the object
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
/// @brief Initialize simulator
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
	callbacks.OnInitialize = FWAS_EVDS_Callback_PreInitialize;
	callbacks.OnPostInitialize = FWAS_EVDS_Callback_PostInitialize;
	callbacks.OnDeinitialize = 0;
	EVDS_System_SetGlobalCallbacks(simulator->system,&callbacks);

	//Start simulation thread
	simulator->paused = 1;
	SIMC_Thread_Create(FWAS_EVDS_Simulate_Thread,simulator);
	return 1;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Deinitialize simulator
////////////////////////////////////////////////////////////////////////////////
void FWAS_Deinitialize(FWAS* simulator) {

}