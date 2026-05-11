#ifndef COLLISION_H
#define COLLISION_H

// Function that should be called by external logic
// Takes pointers to two different objects as input and determines if they are close enough to collide
// If they are, then the objects are passed to hibox_collide() to perform coordinate space transformations
// And verify that the hitboxes are overlapping.
int check_collision_objects(struct game_object *, struct game_object *);

// Transforms the coordinate space of one object into the other so it may be determined if one has a vertex within the other
// Called by check_collision_objects() with the proper arguments
int hitbox_collide(struct game_object *, struct gl_hitbox *, struct game_object *, struct gl_hitbox *);

// Allocates twice the size of the largest number of vertices of the arguments given
// That way 2 of any object can be stored in the allocated memory
void allocate_collision_memory(size_t, size_t);

// Must be called after a call to any collision check
void free_collision_memory();

#endif
