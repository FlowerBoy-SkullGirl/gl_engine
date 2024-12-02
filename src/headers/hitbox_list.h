#ifndef HITBOX_LIST_H
#define HITBOX_LIST_H

struct hitbox_list{
	struct gl_hitbox *data;
	struct hitbox_list *next;
}

struct hitbox_list *create_hitbox_node();

struct hitbox_list *append_hitbox(struct hitbox_list *, struct gl_hitbox *);

struct hitbox_list *destroy_hitbox_list(struct hitbox_list *);

struct hitbox_list *access_hb_list_index(struct hitbox_list *, int);

#endif
