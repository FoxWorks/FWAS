////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "xp_fwas.h"


////////////////////////////////////////////////////////////////////////////////
/// Draw a mesh in 3D world
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_DrawMesh(EVDS_MESH* mesh) {
	//int i,v;

	glVertexPointer(3, GL_FLOAT, 0, mesh->vertices);
	glNormalPointer(GL_FLOAT, 0, mesh->normals);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, mesh->indices);
	glEnable(GL_CULL_FACE);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);

	/*glBegin(GL_LINES);
	for (i = 0; i < mesh->num_triangles; i++) {
		EVDS_MESH_TRIANGLE* triangle = &mesh->triangles[i];
		if (triangle->thickness == 0.0) continue;
		for (v = 0; v < 3; v++) {
			glColor3f(1.0f,1.0f,1.0f);
			glVertex3f(triangle->vertex[v].x,triangle->vertex[v].y,triangle->vertex[v].z);
			glVertex3f(triangle->vertex[(v+1)%3].x,triangle->vertex[(v+1)%3].y,triangle->vertex[(v+1)%3].z);
		}
	}
	glEnd();*/
}


////////////////////////////////////////////////////////////////////////////////
/// Draw an object
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_DrawObject(EVDS_OBJECT* object) {
	FWAS_EVDS_USERDATA* userdata;
	int i,j,lod;
	float opengl_matrix[16];
	EVDS_MATRIX Qmatrix;
	EVDS_STATE_VECTOR vector;
	SIMC_LIST* list;
	SIMC_LIST_ENTRY* entry;

	//Enter local transformation
	glPushMatrix();

	//Transform to current vessels coordinates or planetary coordinates
	EVDS_Object_GetStateVector(object,&vector);
	if (vector.position.coordinate_system == earth) {
		double x,y,z;
		EVDS_GEODETIC_COORDINATE geocoord;
		EVDS_Geodetic_FromVector(&geocoord,&vector.position,0);
		XPLMWorldToLocal(geocoord.latitude,geocoord.longitude,geocoord.elevation,&x,&y,&z);
		glTranslatef((float)x,(float)y,(float)z);
		glRotatef( 90.0f, 0,1,0);
		glRotatef(180.0f, 0,1,0);
		glRotatef(-90.0f, 1,0,0);
	} else {
		glTranslatef((float)vector.position.x,(float)vector.position.y,(float)vector.position.z);
	}

	//Get quaternion and convert it to OpenGL
	EVDS_Quaternion_ToMatrix(&vector.orientation,Qmatrix);
	for (i=0; i<4; i++) { //Transpose matrix for OpenGL
		for (j=0; j<4; j++) {
			opengl_matrix[i*4+j] = (float)Qmatrix[i*4+j];//(float)Qmatrix[j*4+i];
		}
	}
	glMultMatrixf(opengl_matrix);

	//Render objects mesh data
	EVDS_Object_GetUserdata(object,&userdata);
	if (userdata) {
		//Get bounding box of the object
		lod = FWAS_LOD_LEVELS;
		if (userdata->lod_count > 0) {
			XPFWAS_DrawMesh(userdata->mesh[userdata->lod_count-1]);
		}

		//Draw mesh
		//XPFWAS_DrawMesh(userdata->mesh);
	}

	//Draw children
	EVDS_Object_GetChildren(object,&list);
	entry = SIMC_List_GetFirst(list);
	while (entry) {
		XPFWAS_DrawObject(SIMC_List_GetData(list,entry));
		entry = SIMC_List_GetNext(list,entry);
	}

	//Leave local transformation
	glPopMatrix();
}