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
#include "headers/objects.h"

#define DEFAULT_R_MOMENTUM 0.7

// Allocates memory for a game_object with malloc, must be freed with free_game_object()
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

// Frees memory allocated by init_game_object()
// Does not free mesh object, since a mesh can be used for many object's
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

int set_object_vel(struct game_object *op, struct velocity vel)
{
	if (op == NULL)
		return 1;

	(op->vel.x) = vel.x;
	(op->vel.y) = vel.y;
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

// Draws the game object by passing uniforms to the shaders and calling the glDrawElements function
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
