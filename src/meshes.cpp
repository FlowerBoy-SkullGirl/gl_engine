#include <iostream>
#include <stdlib.h>
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "headers/buffers.h"
#include "headers/meshes.h"
#include "headers/shapes.h"

#define MAX_DIGIT_MESH 256
#define SHADER_LAYOUTS 2

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
	// Return null if unable to allocate memory
	if (buffer == NULL)
		return NULL;
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

	// Buffer will always be non-null at this point of execution
	free(buffer);

	// fp will always be non-null at this point of execution
	fclose(fp);

	return mesh;
}

struct gl_mesh *init_mesh(struct gl_shape *sp)
{
	struct gl_mesh *mp = (struct gl_mesh *)malloc(sizeof(struct gl_mesh));
	mp->shape = sp;
	mp->VBO = NULL;
	mp->EBO = NULL;
	mp->VAO = 0;
	mp->num_indices = 0;

	return mp;
}

int build_buffers(struct gl_mesh *mp)
{
	if (mp == NULL)
		return 1;
	if (mp->shape == NULL)
		return 1;

	struct buffer_data mesh_buf = pack_data((mp->shape)->vertices, sizeof(float) * (mp->shape)->size_v, (mp->shape)->size_v, GL_FLOAT);
	struct buffer_data index_buf = pack_data((mp->shape)->indices, sizeof(float) * (mp->shape)->size_i, (mp->shape)->size_i, GL_FLOAT);

	mp->num_indices = index_buf.elements;
	// Create the glBuffers
	mp->VBO = init_buffer(GL_ARRAY_BUFFER);
	mp->EBO = init_buffer(GL_ELEMENT_ARRAY_BUFFER);

	// Create the object's VAO
	mp->VAO = init_array();
	bind_buffer(mp->VBO);
	bind_buffer(mp->EBO);

	// Load the data into the buffers
	set_buffer(mp->VBO, mesh_buf, GL_DYNAMIC_DRAW);
	set_buffer(mp->EBO, index_buf, GL_DYNAMIC_DRAW);

	// Set the array attributes for the vertex shader
	int num_layouts = SHADER_LAYOUTS;
	set_array_attributes(0, 2, GL_FLOAT, sizeof(float), num_layouts, 0);
	set_array_attributes(1, 2, GL_FLOAT, sizeof(float), num_layouts, 2);

	bind_array(0);
	return 0;

}

void destroy_mesh(struct gl_mesh *mp)
{
	if (mp == NULL)
		return ;
	if (mp->shape != NULL)
		destroy_gl_shape(mp->shape);
	if (mp->VBO != NULL)
		delete_buffer(mp->VBO);
	if (mp->EBO != NULL)
		delete_buffer(mp->EBO);
	free(mp);
}
