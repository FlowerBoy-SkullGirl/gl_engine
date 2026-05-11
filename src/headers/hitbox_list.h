#ifndef HITBOX_LIST_H
#define HITBOX_LIST_H

// A pointer to an individual hitbox and a pointer to the next list node
// A singly linked list
struct hitbox_list{
	struct gl_hitbox *data;
	struct hitbox_list *next;
};

// Functions used to create, add, destroy, and access the linked list nodes
struct hitbox_list *create_hitbox_list_node();

struct hitbox_list *append_hitbox(struct hitbox_list *, struct gl_hitbox *);

struct hitbox_list *destroy_hitbox_list(struct hitbox_list *);

struct hitbox_list *access_hb_list_index(struct hitbox_list *, int);

#endif
