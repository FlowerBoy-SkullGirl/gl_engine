#include "headers/vectors.h" // all our mathematical functions
#include "headers/hitbox.h"
#include "headers/hitbox_list.h"
#include "headers/objects.h"
#include "headers/object_list.h"

#define DEFAULT_MESH_SIZE 2.0
float g_mesh_max_size = sqrt(DEFAULT_MESH_SIZE);

int check_collision_objects(struct game_object *ob1, struct game_object *ob2)
{
	int found_collision = 0;
	int box1 = 0;
	int box2 = 0;
	// Check if the objects are near enough to collide
	double longest_d = (max_dimension(ob1->scale_x, ob1->scale_y) * g_mesh_max_size) + (max_dimension(ob2->scale_x, ob2->scale_y) * g_mesh_max_size);
	if (distance2(ob1->pos_x, ob1->pos_y, ob2->pos_x, ob2->pos_y) < longest_d)
		return 0;

	for (int i = 0; access_hb_list_index(ob1->hb_list, i) != NULL; i++){
		for (int j = 0; access_hb_list_index(ob2->hb_list, j) != NULL; j++){
			if (hitbox_collide(ob1, (ob1->hb_list)[i], ob2, (ob2->hb_list)[j])){
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


}

int hitbox_collide(struct game_object *ob1, struct gl_hitbox *hb1, struct game_object *ob2, struct gl_hitbox *hb2)
{
	// Add the angles of the object and their relative hitbox and use unit circle ranges
	double r1 = fmodl((ob1->rotation) + (hb1->offset_rot), G_PI * 2.0f);
	struct velocity p1;
	p1.x = (ob1->pos_x) + (hb1->offset_x);
	p1.y = (ob1->pos_y) + (hb1->offset_y);

	double r2 = fmodl((ob2->rotation) + (hb2->offset_rot), G_PI * 2.0f);
	struct velocity p2;
	p2.x = (ob2->pos_x) + (hb2->offset_x);
	p2.y = (ob2->pos_y) + (hb2->offset_y);

	// Check if the objects are near enough to collide
	double longest_d = (max_dimension(hb1->scale_x, hb1->scale_y) * g_mesh_max_size) + (max_dimension(hb2->scale_x, hb2->scale_y) * g_mesh_max_size);
	if (distance2(p1->pos_x, p1->pos_y, p2->pos_x, p2->pos_y) < longest_d)
		return 0;

	// Find the difference between their rotation so that hb2 can be transformed into hb1 space
	double delta_r = r1 - r2;
	// Apply the necessary transformations to the vertices to determine their edges
	float *vert1 = (float *)malloc((sizeof(float) * (hb1->mesh->shape->size_v)));
	float *vert2 = (float *)malloc((sizeof(float) * (hb2->mesh->shape->size_v)));

	for (int i = 0; i < (hb1->mesh->shape->size_v); i += 2){
		*(vert1 + i) = *((hb1->mesh->shape)->vertices + i);
		*(vert1 + i + 1) = *((hb1->mesh->shape)->vertices + i + 1);
	}
	
	for (int i = 0; i < (hb2->mesh->shape->size_v); i += 2){
		*(vert2 + i) = *((hb2->mesh->shape)->vertices + i);
		*(vert2 + i + 1) = *((hb2->mesh->shape)->vertices + i + 1);
		//rotating vert2 into vert1 space
		rotate2((vert2 + i), (vert2 + i + 1), delta_r);
	}
	
	// TODO: Transform the vertices into 'hb1 space' relative to its rotation
	// Determine if hitbox 2 has a vertex inside hitbox 1 using barycentric coordinates
	
	free(vert1);
	free(vert2);
}
