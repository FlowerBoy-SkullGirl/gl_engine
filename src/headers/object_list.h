#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

struct object_list{
	struct game_object *op;
	struct object_list *next;
};

struct object_list *create_object_list_node();

struct object_list *append_object(struct object_list *, struct game_object *);

struct object_list *destroy_object_list(struct object_list *);

struct object_list *access_go_list_index(struct object_list *, int);

#endif
