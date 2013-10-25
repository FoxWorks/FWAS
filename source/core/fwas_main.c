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
	EVDS_Mesh_Generate(object,&userdata->mesh,0.1f,0);

	//Set as userdata
	EVDS_Object_SetUserdata(object,userdata);
	return EVDS_OK;
}


////////////////////////////////////////////////////////////////////////////////
/// Initialize simulator
////////////////////////////////////////////////////////////////////////////////
int FWAS_Initialize(FWAS** p_simulator) {
	FWAS* simulator;
	EVDS_OBJECT* root;
	EVDS_GLOBAL_CALLBACKS callbacks = { 0 };

	//Create new simulator
	simulator = malloc(sizeof(FWAS));
	*p_simulator = simulator;
	if (!simulator) return 0;
	memset(simulator,0,sizeof(FWAS));

	//Initialize EVDS
	EVDS_System_Create(&simulator->system);
	EVDS_Common_Register(simulator->system);

	//Set proper callbacks
	callbacks.OnInitialize = 0;
	callbacks.OnPostInitialize = FWAS_EVDS_Callback_PostInitialize;
	callbacks.OnDeinitialize = 0;
	EVDS_System_SetGlobalCallbacks(simulator->system,&callbacks);

	EVDS_System_GetRootInertialSpace(simulator->system,&root);	
	EVDS_Object_LoadFromFile(root,"./Resources/plugins/fwas_x-plane/testvehicle.evds",&simulator->test);
	EVDS_Object_Initialize(simulator->test,1);
	return 1;

}


////////////////////////////////////////////////////////////////////////////////
/// Deinitialize simulator
////////////////////////////////////////////////////////////////////////////////
void FWAS_Deinitialize(FWAS* simulator) {

}