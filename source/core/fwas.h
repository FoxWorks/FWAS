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




/// Callback when FWAS logs a message
typedef void FWAS_Callback_Log(int level, char* message, ...);
/// Information message
#define FWAS_MESSAGE_INFO		0
/// Warning message
#define FWAS_MESSAGE_WARNING	1
/// Error message
#define FWAS_MESSAGE_ERROR		2


////////////////////////////////////////////////////////////////////////////////
/// @brief FoxWorks Aerospace Simulator state
////////////////////////////////////////////////////////////////////////////////
typedef struct FWAS_TAG {
	EVDS_SYSTEM* system;
	EVDS_OBJECT* active_vessel;
	FWAS_Callback_Log* log;
} FWAS;


////////////////////////////////////////////////////////////////////////////////
/// @brief Userdata for the EVDS object
////////////////////////////////////////////////////////////////////////////////
typedef struct FWAS_EVDS_USERDATA_TAG {
	EVDS_OBJECT* object;
	EVDS_MESH* mesh;
} FWAS_EVDS_USERDATA;




////////////////////////////////////////////////////////////////////////////////
/// Initialize simulator
int FWAS_Initialize(FWAS** p_simulator);
/// Set logging callback
void FWAS_SetCallback_Log(FWAS* simulator, FWAS_Callback_Log* onLog);

/// Deinitialize simulator
void FWAS_Deinitialize(FWAS* simulator);

/// Set Earth and Moon simulation scene
void FWAS_SetScene_EarthMoon(FWAS* simulator);

/// Load a vessel from file
EVDS_OBJECT* FWAS_Vessel_Load(FWAS* simulator, char* filename);
/// Load a vessel from file and place it somewhere on the planet
EVDS_OBJECT* FWAS_Vessel_LoadAndPlace(FWAS* simulator, EVDS_GEODETIC_COORDINATE* location, char* filename);
/// Set active vessel
void FWAS_Vessel_SetActive(FWAS* simulator, EVDS_OBJECT* vessel);


////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
#endif
