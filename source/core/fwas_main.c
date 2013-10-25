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


/// Initialize simulator
int FWAS_Initialize(FWAS** p_simulator) {
	FWAS* simulator;

	//Create new simulator
	simulator = malloc(sizeof(FWAS));
	*p_simulator = simulator;
	if (!simulator) return 0;
	memset(simulator,0,sizeof(FWAS));

	//Initialize EVDS
	EVDS_System_Create(&simulator->system);
	return 1;

}

/// Deinitialize simulator
void FWAS_Deinitialize(FWAS* simulator) {

}