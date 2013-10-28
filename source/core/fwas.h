////////////////////////////////////////////////////////////////////////////////
/// @file
///
/// @brief FoxWorks Aerospace Simulator
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
#ifndef FWAS_H
#define FWAS_H
#ifdef __cplusplus
extern "C" {
#endif

// External Vessel Dynamics Simulator
#include "evds.h"
#include "evds_train_wheels.h"
// Internal Vessel Systems Simulator
//#include "ivss.h"
// Realtime Digital Radio Simulator
//#include "rdrs.h"




////////////////////////////////////////////////////////////////////////////////
/// Callbacks
////////////////////////////////////////////////////////////////////////////////
/// Callback when FWAS logs a message
typedef void FWAS_Callback_Log(int level, char* message, ...);
/// Callback for a single vessel
typedef void FWAS_Callback_Vessel(EVDS_OBJECT* vessel);


/// Information message
#define FWAS_INFO			0
/// Warning message
#define FWAS_WARNING		1
/// Error message
#define FWAS_ERROR			2


/// Number of LODs for the object meshes
#define FWAS_LOD_LEVELS		6




////////////////////////////////////////////////////////////////////////////////
/// @brief FoxWorks Aerospace Simulator state
////////////////////////////////////////////////////////////////////////////////
typedef struct FWAS_TAG {
	int paused;						///< Is system paused
	EVDS_SYSTEM* system;			///< EVDS system
	EVDS_OBJECT* active_vessel;		///< Currently actively selected vessel

	//List of callbacks
	FWAS_Callback_Log* log;
} FWAS;


////////////////////////////////////////////////////////////////////////////////
/// @brief Userdata for the EVDS object
////////////////////////////////////////////////////////////////////////////////
typedef struct FWAS_EVDS_USERDATA_TAG {
	EVDS_OBJECT* object;
	EVDS_MESH* mesh[FWAS_LOD_LEVELS];
	int lod_count;
} FWAS_EVDS_USERDATA;




////////////////////////////////////////////////////////////////////////////////
/// Simulator management
////////////////////////////////////////////////////////////////////////////////
/// Initialize simulator
int FWAS_Initialize(FWAS** p_simulator);
/// Deinitialize simulator
void FWAS_Deinitialize(FWAS* simulator);


////////////////////////////////////////////////////////////////////////////////
/// Default scenes
////////////////////////////////////////////////////////////////////////////////
/// Load Earth and Moon simulation scene
void FWAS_LoadScene_EarthMoon(FWAS* simulator);
/// Set Solar system simulation scene
void FWAS_LoadScene_SolarSystem(FWAS* simulator);


////////////////////////////////////////////////////////////////////////////////
/// Vessel and scenery (building) management
////////////////////////////////////////////////////////////////////////////////
/// Load object(s) from file
EVDS_OBJECT* FWAS_Object_LoadFromFile(FWAS* simulator, EVDS_OBJECT* parent, char* filename);
/// Load object(s) from string
//EVDS_OBJECT* FWAS_Object_LoadFromString(FWAS* simulator, EVDS_OBJECT* parent, char* description);

/// Iterate across every vessel in parent
void FWAS_Object_IterateChildren(FWAS* simulator, EVDS_OBJECT* parent, FWAS_Callback_Vessel* callback);
/// Iterate across every vessel in parent (asynchronous)
//void FWAS_Object_AsyncIterateChildren(FWAS* simulator, EVDS_OBJECT* parent, FWAS_Callback_Vessel* callback);
/// Iterate across every object of given type
//void FWAS_Object_IterateByType(FWAS* simulator, char* type, FWAS_Callback_Vessel* callback);
/// Iterate across every object of given type (asynchronous)
//void FWAS_Object_AsyncIterateByType(FWAS* simulator, char* type, FWAS_Callback_Vessel* callback);

/// Get object by name
EVDS_OBJECT* FWAS_Object_GetByName(FWAS* simulator, char* name);

/// Set active vessel
void FWAS_Object_SetActiveVessel(FWAS* simulator, EVDS_OBJECT* vessel);


////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif
