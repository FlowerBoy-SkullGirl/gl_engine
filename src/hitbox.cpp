#include "headers/hitbox.h"
#include "stdlib.h"

// Allocates memory for a hitbox, which must be freed with destroy_hitbox()
struct gl_hitbox *create_hitbox(struct gl_mesh *mp)
{
	if (mp == NULL)
		return NULL;
	struct gl_hitbox *hb = (struct gl_hitbox *)malloc(sizeof(struct gl_hitbox));
	hb->mesh = mp;

	hb->offset_x = 0.0f;
	hb->offset_y = 0.0f;

	hb->scale_x = 1.0f;
	hb->scale_y = 1.0f;

	hb->offset_rot = 0.0f;

	return hb; 
}

// Frees memory from a hitbox, which was allocated with create_hitbox()
void destroy_hitbox(struct gl_hitbox *hb)
{
	if (hb == NULL)
		return;
	free(hb);
}
