#include <iostream>
#include <stdlib.h>

#include "headers/meshes.h"
#include "headers/shapes.h"

#define MAX_DIGIT_MESH 256

struct gl_shape *load_mesh(const char *filen)
{
	FILE *fp = fopen(filen, "r");
	if (fp == NULL)
		return NULL;	

	struct gl_shape *mesh = (struct gl_shape *)malloc(sizeof(struct gl_shape));
	mesh->size_v = 0;
	mesh->size_i = 0;
	// Determine number of elements
	int encountered_semi = 0;
	int encountered_comment = 0;
	char c;
	while ((c = fgetc(fp)) != EOF){
		if (c == '\n'){
			encountered_comment = 0;
		}
		if (encountered_comment)
			continue;
		if (c == '/'){
			encountered_comment = 1;
		}
		if (c == ';'){
			encountered_semi = 1;
		}

		if (c == ','){
			if (encountered_semi){
				mesh->size_i += 1;
				continue;
			}
			mesh->size_v += 1;
		}
	}
	// Adjust the count for the final value not followed by a comma
	mesh->size_v += 1;
	mesh->size_i += 1;

	// Build gl_shape arrays
	mesh->vertices = (float *)malloc(sizeof(float) * (mesh->size_v));
	mesh->indices = (unsigned int *)malloc(sizeof(unsigned int) * (mesh->size_i));

	// Return to start of file
	fseek(fp, SEEK_SET, 0);
	encountered_comment = 0;
	encountered_semi = 0;
	
	char *buffer = (char *)malloc(MAX_DIGIT_MESH);
	int size_buf = 0;
	int i = 0;
	int j = 0;
	int sign = 0;
	while ((c = fgetc(fp)) != EOF){
		if (c == '\n'){
			encountered_comment = 0;
			continue;
		}
		if (encountered_comment)
			continue;
		if (c == '/'){
			encountered_comment = 1;
			continue;
		}
		if (c == '-')
			sign = 1;
		if (c == ';'){
			// The last vertex that has no comma
			*(buffer + size_buf) = '\0';
			sscanf(buffer, "%f", ((mesh->vertices) + i));
			size_buf = 0;
			encountered_semi = 1;
			sign = 0;
			continue;
		}
		if (c == ','){
			*(buffer + size_buf) = '\0';
			size_buf = 0;
			if (encountered_semi){
				sscanf(buffer, "%d", ((mesh->indices) + j));
				if (sign)
					*(mesh->indices + j) *= -1;
				j++;
				sign = 0;
					
				continue;
			}
			sscanf(buffer, "%f", ((mesh->vertices) + i));
			if (sign)
				*(mesh->vertices + i) *= -1.0f;
			i++;
			sign = 0;
			continue;
		}
		if (c == '.' || (c <= '9' && c >= '0')){
			*(buffer + size_buf) = c;
			size_buf++;
		}
	}
	// The last index should have no comma and not yet been recorded
	sscanf(buffer, "%d", ((mesh->indices) + j));

	free(buffer);

	return mesh;
}
