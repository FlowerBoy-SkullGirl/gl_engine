#include <iostream>
#include <stdlib.h>
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "headers/shapes.h"
#include "headers/rgba.h"
#include "headers/meshes.h"
#include "headers/hitbox.h"
#include "headers/hitbox_list.h"
#include "headers/buffers.h"
#include "headers/textures.h"
#include "headers/uniforms.h"
#include "headers/vectors.h" // velocity struct
#include "headers/database.h"
#include "headers/objects.h"

#define DEFAULT_R_MOMENTUM 0.7

#define SERIAL_NUM_INTS = 1;
#define SERIAL_NUM_FLOATS = 5;
#define SERIAL_NUM_STRINGS = 0;

struct game_object *init_game_object()
{
	struct game_object *op = (struct game_object *)malloc(sizeof(struct game_object));

	op->hb_list = create_hitbox_list_node();

	op->mesh = NULL;
	op->color = convert_to_rgba(1.0f, 1.0f, 1.0f, 1.0f);
	op->texture = 0;


	op->rotation = 0.0f;
	op->pos_x = 0.0f;
	op->pos_y = 0.0f;
	op->scale_x = 0.0f;
	op->scale_y = 0.0f;

	op->mass = 0.0f;
	(op->vel).x = 0.0f;
	(op->vel).y = 0.0f;
	op->r_momentum = DEFAULT_R_MOMENTUM;
	
	op->use_tex = 0;
	op->use_hitbox = 0;

	return op;
}

// Does not free mesh object
void free_game_object(struct game_object *op)
{
	if (op == NULL)
		return;

	destroy_hitbox_list(op->hb_list);
	free(op);
}

int set_object_mesh(struct game_object *op, struct gl_mesh *mesh)
{
	if (mesh == NULL || op == NULL)
		return 1;

	op->mesh = mesh;
	return 0;
}

int set_object_color(struct game_object *op, struct rgba color)
{
	if (op == NULL)
		return 1;

	op->color = color;
	return 0;
}

int set_object_texture(struct game_object *op, unsigned int tex_id)
{
	if (op == NULL)
		return 1;

	op->texture = tex_id;
	op->use_tex = 1;
	return 0;
}

int set_object_mass(struct game_object *op, float m)
{
	if (op == NULL)
		return 1;

	op->mass = m;
	return 0;
}

// Uniforms are not immediately set until object is ready to be drawn
int set_object_rotation(struct game_object *op, float rotation)
{
	if (op == NULL)
		return 1;
	op->rotation = rotation;
	return 0;
}

int set_object_pos(struct game_object *op, float x, float y)
{
	if (op == NULL)
		return 1;

	op->pos_x = x;
	op->pos_y = y;
	return 0;
}

int set_object_scale(struct game_object *op, float x, float y)
{
	if (op == NULL)
		return 1;

	op->scale_x = x;
	op->scale_y = y;
	return 0;
}

int add_object_hitbox(struct game_object *op, struct gl_hitbox *hb)
{
	if (op == NULL || hb == NULL)
		return 1;

	append_hitbox(op->hb_list, hb);
	op->use_hitbox = 1;
	return 0;
}

// Wraps calls to OpenGL and sets uniform values for the appropriate shaders
void draw_game_object(struct game_object *op, unsigned int shader)
{
	// Set the uniforms
	set_uniforms1(op->rotation, shader, "rotationRad");
	set_uniforms2(op->scale_x, op->scale_y, shader, "scalars");
	set_uniforms2(op->pos_x, op->pos_y, shader, "translation");
	// Fragment uniforms
	if (!(op->use_tex)){
		set_uniforms4((op->color).r, (op->color).g, (op->color).b, (op->color).a, shader, "color");
	}
	// Set the texture 
	if (op->use_tex){
		bind_texture(op->texture);
	}
	set_uniform_int(op->use_tex, shader, "useTexture");

	// Bind the VAO
	bind_array((op->mesh)->VAO);
	glDrawElements(GL_TRIANGLES, (op->mesh)->num_indices, GL_UNSIGNED_INT, 0);
}

// Write data into a buffer, checking if the specified size and offset will write outside of the bounds of the buffer
// size_t is unsigned, so we do not worry about negative offset or obj size arguments
size_t write_to_buffer(char *buffer, char *data, size_t offset, size_t size_obj, size_t buf_len)
{
	if (offset + size_obj > buf_len)
		return 0;
	memcpy(buffer + offset, data, size_obj);
	return size_obj;
}

// Take all relevant values within the game object and serialize them into a non-typed array of data
// Adhering to struct row_object specifications
struct row_object *serialize_game_object(struct game_object *op)
{
	if (op == NULL)
		return NULL;
	if (op->mesh == NULL)
		return NULL;

	struct row_object *ro = (struct row_object *) malloc(sizeof(struct row_object));
	if (ro == NULL)
		return NULL;

	// Num of all elements to be added to the data structure
	ro->column_count = SERIAL_NUM_INTS + SERIAL_NUM_FLOATS + SERIAL_NUM_STRINGS;
	// Allocate memory for the data type list
	ro->data_type_list = (enum DB_TYPES *) malloc ((ro->column_count) * sizeof(enum DB_TYPES));

	// Iterate through the data type list, first adding integers, then floats, then strings
	int i = 0;
	for(; i < column_count ; i++){
		if (i < SERIAL_NUM_INTS)
			*(ro->data_type_list) = DB_INT;
		if (i >= SERIAL_NUM_INTS && i < SERIAL_NUM_INTS + SERIAL_NUM_FLOATS)
			*(ro->data_type_list) = DB_FLOAT;
		if (i >= SERIAL_NUM_INTS + SERIAL_NUM_FLOATS)
			*(ro->data_type_list) = DB_STRING;
	}

	// Allocate the proper amount of memory for the data_list
	size_t int_size = sizeof(int);
	size_t float_size = sizeof(float);
	ro->data_list_size = 0;
	ro->data_list_size += SERIAL_NUM_INTS * int_size;
	ro->data_list_size += SERIAL_NUM_FLOATS * float_size;

	// String memory has to be calculated per string, therefore depends on which strings are being stored
	// This class currently stores 0 strings
	if (SERIAL_NUM_STRINGS > 0){
		//Insert string storing logic here if SERIAL_NUM_STRINGS is ever implemented
	}

	// Call malloc
	ro->data_list = malloc(ro->data_list_size);

	// Cast a pointer to easily write memory to the void * type data_list
	char *dp = (char *) ro->data_list;

	// Begin writing the data into the buffer
	size_t return_check = 0;
	size_t offset = 0;
	enum GL_MeshType object_mesh;
	// There are 3 indices in a triangle mesh and 6 in a square mesh, the only two meshes currently defined in the project
	if (op->mesh->num_indices == 0){
		free_serialized_data(ro);
		return NULL;
	}
	if (op->mesh->num_indices == 3)
		object_mesh = TriangleGLMesh;
	if (op->mesh->num_indices == 6)
		object_mesh = SquareGLMeshGLMesh;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &object_mesh, offset, int_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &(op->rotation), offset, float_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &(op->pos_x), offset, float_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &(op->pos_y), offset, float_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &(op->scale_x), offset, float_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(dp, &(op->scale_y), offset, float_size, ro->data_list_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment offset by the size of the data that was written
	offset += return_check;

	// All data has been written and sizes have been specified, row_object can be returned
	return ro;
}

struct game_object *deserialize_game_object(struct row_object *ro)
{
	if (ro == NULL)
		return NULL;
	if (ro->data_list == NULL)
		return NULL;
	if (ro->data_type_list == NULL)
		return NULL;

	// Verify row_object has the expected number of columns
	if (ro->column_count != SERIAL_NUM_INTS + SERIAL_NUM_FLOATS + SERIAL_NUM_STRINGS)
		return NULL;
	// Verify DB_TYPES list contains expected order and amount of types
	for (int i = 0; i < ro->column_count; i++)
	{
		if (i < SERIAL_NUM_INTS)
			if(ro->data_type_list != DB_INT)
				return NULL;
		if (i >= SERIAL_NUM_INTS && i < SERIAL_NUM_INTS + SERIAL_NUM_FLOATS)
			if(ro->data_type_list != DB_FLOAT)
				return NULL;
		if (i >= SERIAL_NUM_INTS + SERIAL_NUM_FLOATS)
			if(ro->data_type_list != DB_STRING)
				return NULL;
	}
	
	// Allocate memory for the game object
	struct game_object *op = init_game_object();

	// Read data from data_list heap into game_object variables
	char *dp = ro->data_list;
	size_t offset = 0;

	// First data is the mesh type
	enum GL_MeshType object_mesh;
	// Call helper function to write data
	return_check = write_to_buffer(&mesh_object, dp, offset, int_size, int_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(&(op->rotation), dp, offset, float_size, float_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(&(op->pos_x), dp, offset, float_size, float_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(&(op->pos_y), dp, offset, float_size, float_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(&(op->scale_x), dp, offset, float_size, float_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// Call helper function to write data
	return_check = write_to_buffer(&(op->scale_y), dp, offset, float_size, float_size);
	if(!return_check){
		free_serialized_data(ro);
		return NULL;
	}
	// Increment data pointer to align with next data type
	dp += return_check;

	// All variables have been read into the object
	return op;
}

struct row_object *free_serialized_data(struct row_object *ro)
{
	if(ro == NULL)
		return NULL;
	if(ro->data_list != NULL){
		free(ro->data_list);
		ro->data_list == NULL;
	}
	if(ro->data_type_list != NULL){
		free(ro->data_type_list);
		ro->data_type_list == NULL;
	}

	free(ro);
	ro = NULL;
	return NULL;
}
