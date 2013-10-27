////////////////////////////////////////////////////////////////////////////////
/// @file
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "xp_fwas.h"
#include "xp_matrix.h"




////////////////////////////////////////////////////////////////////////////////
/// Project 3D point onto screen
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_ProjectXYZ(float x, float y, float z, float* u, float* v) {
	float view[16], proj[16], viewProj[16];
	float in[4],out[4];
	glGetFloatv(GL_MODELVIEW_MATRIX,  view);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);

	in[0] = x;
	in[1] = y;
	in[2] = z;
	in[3] = 1.0f;
	mult_matrix(viewProj,view,proj);
	transform_vector(out,viewProj,in);

	*u = out[0];
	*v = out[1];
}


////////////////////////////////////////////////////////////////////////////////
/// Draw a mesh in 3D world
////////////////////////////////////////////////////////////////////////////////
void XPFWAS_DrawMesh(EVDS_MESH* mesh) {
	/*glVertexPointer(3, GL_FLOAT, 0, mesh->vertices);
	glNormalPointer(GL_FLOAT, 0, mesh->normals);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glDisable(GL_CULL_FACE);
	glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, mesh->indices);
	glEnable(GL_CULL_FACE);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);*/

	int i,v;
	glBegin(GL_LINES);
	for (i = 0; i < mesh->num_triangles; i++) {
		EVDS_MESH_TRIANGLE* triangle = &mesh->triangles[i];
		if (triangle->thickness == 0.0) continue;
		for (v = 0; v < 3; v++) {
			glColor3f(1.0f,1.0f,1.0f);
			glVertex3f(triangle->vertex[v].x,triangle->vertex[v].y,triangle->vertex[v].z);
			glVertex3f(triangle->vertex[(v+1)%3].x,triangle->vertex[(v+1)%3].y,triangle->vertex[(v+1)%3].z);
		}
	}
	glEnd();
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
	if (userdata && (userdata->lod_count > 0)) {
		int i,j;
		float visual_size;
		struct { float x,y; } vertices[8];
		EVDS_MESH* mesh = userdata->mesh[0];

		//Get bounding box of the object
		XPFWAS_ProjectXYZ(mesh->bbox_min.x,mesh->bbox_min.y,mesh->bbox_min.z,&vertices[0].x,&vertices[0].y);
		XPFWAS_ProjectXYZ(mesh->bbox_max.x,mesh->bbox_min.y,mesh->bbox_min.z,&vertices[1].x,&vertices[1].y);
		XPFWAS_ProjectXYZ(mesh->bbox_min.x,mesh->bbox_max.y,mesh->bbox_min.z,&vertices[2].x,&vertices[2].y);
		XPFWAS_ProjectXYZ(mesh->bbox_max.x,mesh->bbox_max.y,mesh->bbox_min.z,&vertices[3].x,&vertices[3].y);

		XPFWAS_ProjectXYZ(mesh->bbox_min.x,mesh->bbox_min.y,mesh->bbox_max.z,&vertices[4].x,&vertices[4].y);
		XPFWAS_ProjectXYZ(mesh->bbox_max.x,mesh->bbox_min.y,mesh->bbox_max.z,&vertices[5].x,&vertices[5].y);
		XPFWAS_ProjectXYZ(mesh->bbox_min.x,mesh->bbox_max.y,mesh->bbox_max.z,&vertices[6].x,&vertices[6].y);
		XPFWAS_ProjectXYZ(mesh->bbox_max.x,mesh->bbox_max.y,mesh->bbox_max.z,&vertices[7].x,&vertices[7].y);

		//Compute furthest distance between any two points
		visual_size = 0.0;
		for (i=0;i<8;i++) {
			for (j=i;j<8;j++) {
				float distance = 
					(vertices[i].x-vertices[j].x)*(vertices[i].x-vertices[j].x)+
					(vertices[i].y-vertices[j].y)*(vertices[i].y-vertices[j].y);
				if (distance > visual_size) visual_size = distance;
			}
		}
		visual_size = sqrtf(visual_size);

		//Compute LOD level
		lod = (int)(FWAS_LOD_LEVELS*visual_size*2.0f); //>1/2th of screen -> full LOD
		if (lod < 0) lod = 0;
		if (lod > FWAS_LOD_LEVELS-1) lod = FWAS_LOD_LEVELS-1;
		if (lod > userdata->lod_count-1) lod = userdata->lod_count-1;

		//Draw mesh with corresponding LOD
		XPFWAS_DrawMesh(userdata->mesh[lod]);
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