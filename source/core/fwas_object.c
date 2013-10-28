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
/// Load vessel(s) from file
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Object_LoadFromFile(FWAS* simulator, EVDS_OBJECT* parent, char* filename) {
	char reference[1025] = { 0 };
	EVDS_OBJECT* vessel;
	EVDS_Object_GetReference(parent,0,reference,1024);
	simulator->log(FWAS_INFO,"FWAS_Object_Load: Load from %s into %s...",filename,reference);

	//Load the vessel
	EVDS_Object_LoadFromFile(parent,filename,&vessel);
	EVDS_Object_Initialize(vessel,0);
	return vessel;
}


////////////////////////////////////////////////////////////////////////////////
/// Iterate through every child of the parent
////////////////////////////////////////////////////////////////////////////////
void FWAS_Object_IterateChildren(FWAS* simulator, EVDS_OBJECT* parent, FWAS_Callback_Vessel* callback) {
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
/// Get planet by name
////////////////////////////////////////////////////////////////////////////////
EVDS_OBJECT* FWAS_Object_GetByName(FWAS* simulator, char* name) {
	EVDS_OBJECT* planet;
	if (EVDS_System_GetObjectByName(simulator->system,name,0,&planet) != EVDS_OK) {
		simulator->log(FWAS_ERROR,"FWAS_Object_GetByName: Not found: %s",name);
		return 0;
	} else {
		return planet;
	}
}


////////////////////////////////////////////////////////////////////////////////
/// Set active vessel
////////////////////////////////////////////////////////////////////////////////
void FWAS_Object_SetActiveVessel(FWAS* simulator, EVDS_OBJECT* vessel) {
	simulator->active_vessel = vessel;	
}