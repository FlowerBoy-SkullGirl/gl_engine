#include "headers/vectors.h" // all our mathematical functions
#include "headers/rgba.h"
#include "headers/shapes.h"
#include "headers/meshes.h"
#include "headers/hitbox.h"
#include "headers/hitbox_list.h"
#include "headers/objects.h"
#include "headers/object_list.h"
#include <cmath>

#define G_PI 3.1415

#define DEFAULT_MESH_SIZE 2.0
float g_mesh_max_size = sqrt(DEFAULT_MESH_SIZE);

// Transforms the coordinate space of one object into the other so it may be determined if one has a vertex within the other
int hitbox_collide(struct game_object *ob1, struct gl_hitbox *hb1, struct game_object *ob2, struct gl_hitbox *hb2)
{
	int collision_detected = 0;
	// Add the angles of the object and their relative hitbox and use unit circle ranges
	double r1 = fmod((ob1->rotation) + (hb1->offset_rot), G_PI * 2.0f);
	struct velocity p1;
	p1.x = (ob1->pos_x) + (hb1->offset_x);
	p1.y = (ob1->pos_y) + (hb1->offset_y);

	double r2 = fmod((ob2->rotation) + (hb2->offset_rot), G_PI * 2.0f);
	struct velocity p2;
	p2.x = (ob2->pos_x) + (hb2->offset_x);
	p2.y = (ob2->pos_y) + (hb2->offset_y);

	// Find the difference between their rotation so that hb2 can be transformed into hb1 space
	double delta_r = r2 - r1;
	double delta_s_x = (hb2->scale_x) / (hb1->scale_x);
	double delta_s_y = (hb2->scale_y) / (hb1->scale_y);
	float delta_pos_x = (p2.x) - (p1.x);
	float delta_pos_y = (p2.y) - (p1.y);
	scale2(&delta_pos_x, &delta_pos_y, delta_s_x, delta_s_y);
	rotate2(&delta_pos_x, &delta_pos_y, delta_r);
	// Apply the necessary transformations to the vertices to determine their edges
	// TODO: Statically allocate this memory so that we do not call malloc on every collision check
	float *vert1 = (float *)malloc((sizeof(float) * (hb1->mesh->shape->size_v)));
	float *vert2 = (float *)malloc((sizeof(float) * (hb2->mesh->shape->size_v)));

	// Hb1 vertices will be normalized to -1, 1
	// Hb2 will be transformed with hb1 space as the basis
	// This allows barycentric coordinates for collision detection
	for (int i = 0; i < (hb2->mesh->shape->size_v); i += 2){
		*(vert2 + i) = *((hb2->mesh->shape)->vertices + i);
		*(vert2 + i + 1) = *((hb2->mesh->shape)->vertices + i + 1);
		// Transform the vertices into 'hb1 space' relative to its scale, rotation, and pos
		scale2((vert2 + i), (vert2 + i + 1), delta_s_x, delta_s_y);
		rotate2((vert2 + i), (vert2 + i + 1), delta_r);
		translate2((vert2 + i), (vert2 + i + 1), delta_pos_x, delta_pos_y);
	}
	
	// Determine if hitbox 2 has a vertex inside hitbox 1 using barycentric coordinates
	// First check if any vertices are within -1, 1 (collision true)
	for (int i = 0; i < (hb2->mesh->shape->size_v); i += 2){
		if( fabs(*(vert2 + i)) <= 1 && fabs(*(vert2 + i + 1)) <= 1){
			collision_detected = 1;
			break;
		}
	}

	//If detected, return early
	if (collision_detected){
		free(vert1);
		free(vert2);
		return 1;
	}

	//Transform hb1 into hb2 space to check if its vertices lie inside hb2
	delta_r = r1 - r2;
	delta_s_x = (hb1->scale_x) / (hb2->scale_x);
	delta_s_y = (hb1->scale_y) / (hb2->scale_y);
	delta_pos_x = (p1.x) - (p2.x);
	delta_pos_y = (p1.y) - (p2.y);
	scale2(&delta_pos_x, &delta_pos_y, delta_s_x, delta_s_y);
	rotate2(&delta_pos_x, &delta_pos_y, delta_r);

	for (int i = 0; i < (hb1->mesh->shape->size_v); i += 2){
		*(vert1 + i) = *((hb1->mesh->shape)->vertices + i);
		*(vert1 + i + 1) = *((hb1->mesh->shape)->vertices + i + 1);
		scale2((vert1 + i), (vert1 + i + 1), delta_s_x, delta_s_y);
		rotate2((vert1 + i), (vert1 + i + 1), delta_r);
		translate2((vert1 + i), (vert1 + i + 1), delta_pos_x, delta_pos_y);
	}

	for (int i = 0; i < (hb1->mesh->shape->size_v); i += 2){
		if( fabs(*(vert1 + i)) <= 1 && fabs(*(vert1 + i + 1)) <= 1){
			collision_detected = 1;
			break;
		}
	}

	//Cleanup
	free(vert1);
	free(vert2);

	if (collision_detected)
		return 1;
	else
		return 0;
}

// Checks if objects are neart to each other, calls hitbox_collide()
// and reporst the results to the calling function
int check_collision_objects(struct game_object *ob1, struct game_object *ob2)
{
	int found_collision = 0;
	int box1 = 0;
	int box2 = 0;
	// Check if the objects are near enough to collide
	double longest_d = (max_dimension(ob1->scale_x, ob1->scale_y) * g_mesh_max_size) + (max_dimension(ob2->scale_x, ob2->scale_y) * g_mesh_max_size);
	if (distance2(ob1->pos_x, ob1->pos_y, ob2->pos_x, ob2->pos_y) > longest_d)
		return 0;

	for (int i = 0; access_hb_list_index(ob1->hb_list, i) != NULL; i++){
		for (int j = 0; access_hb_list_index(ob2->hb_list, j) != NULL; j++){
			if (hitbox_collide(ob1, access_hb_list_index(ob1->hb_list, i)->data, ob2, access_hb_list_index(ob2->hb_list, j)->data)){
				found_collision = 1;
				box1 = i;
				box2 = j;
				break;
			}
		}
		if (found_collision)
			break;
	}

	// Return early if no collision was found
	if (!found_collision)
		return 0;

	//TODO: return an angle instead
	return 1;
}
