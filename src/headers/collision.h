#ifndef COLLISION_H
#define COLLISION_H

int check_collision_objects(struct game_object *, struct game_object *);

int hitbox_collide(struct game_object *, struct gl_hitbox *, struct game_object *, struct gl_hitbox *);

#endif
