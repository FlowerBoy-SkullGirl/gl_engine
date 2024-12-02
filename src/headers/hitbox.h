#ifndef HITBOX_H
#define HITBOX_H

struct gl_hitbox {
	// Shape of the hitbox
	struct gl_mesh *mesh;

	// Position relative to the object that has the hitbox
	float offset_x;
	float offset_y;

	// Size of the hitbox
	float scale_x;
	float scale_y;

	// Rotation offset from main object
	float offset_rot;
};

struct gl_hitbox *create_hitbox(struct gl_mesh *);

void destroy_hitbox(struct gl_hitbox *);
#endif
