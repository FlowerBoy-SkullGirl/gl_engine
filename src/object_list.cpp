#include <iostream>
#include <stdlib.h>

#include "headers/rgba.h"
#include "headers/vectors.h"
#include "headers/objects.h"
#include "headers/object_list.h"

struct object_list *create_object_list_node()
{
	struct object_list *np = (struct object_list *)malloc(sizeof(struct object_list));
	np->op = NULL;
	np->next = NULL;
	return np;
}

struct object_list *append_object(struct object_list*lp, struct game_object *op)
{
	if (lp == NULL){
		lp = create_object_list_node();
		lp->op = op;
		return lp;
	}

	if (lp->op != NULL){
		lp->next = append_object(lp->next, op);
		return lp;
	}

	lp->op = op;
	return lp;
}

struct object_list *destroy_object_list(struct object_list *np)
{
	if (np == NULL)
		return NULL;

	destroy_object_list(np->next);

	if (np->next == NULL){
		free_game_object(np->op);
	}

	free(np);

	return NULL;
}

struct object_list *access_go_list_index(struct object_list *np, int i)
{
	if (np == NULL)
		return NULL;
	if (i == 0)
		return np;
	return access_go_list_index(np->next, (i - 1));
}
