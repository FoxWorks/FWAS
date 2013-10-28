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
/// Load Earth and Moon simulation scene
////////////////////////////////////////////////////////////////////////////////
void FWAS_LoadScene_EarthMoon(FWAS* simulator) {
	EVDS_OBJECT* root;
	EVDS_OBJECT* solar_system;
	simulator->log(FWAS_INFO,"FWAS_SetScene_EarthMoon: loading default scene");

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