#include <iostream>
#include <stdlib.h>

#include "headers/rgba.h"
#include "headers/hitbox.h"
#include "headers/hitbox_list.h"

// Allocates memory for a node in the singly linked list
struct hitbox_list *create_hitbox_list_node()
{
	struct hitbox_list *np = (struct hitbox_list *)malloc(sizeof(struct hitbox_list));
	np->data = NULL;
	np->next = NULL;
	return np;
}

struct hitbox_list *append_hitbox(struct hitbox_list*lp, struct gl_hitbox *hp)
{
	if (lp == NULL){
		lp = create_hitbox_list_node();
		lp->data = hp;
		return lp;
	}

	if (lp->data != NULL){
		lp->next = append_hitbox(lp->next, hp);
		return lp;
	}

	lp->data = hp;
	return lp;
}

// Frees memory from a node in the singly linked list
struct hitbox_list *destroy_hitbox_list(struct hitbox_list *np)
{
	if (np == NULL)
		return NULL;

	destroy_hitbox_list(np->next);

	if (np->next == NULL){
		destroy_hitbox(np->data);
	}

	free(np);

	return NULL;
}

// Iterates through a list, starting at np until the index is 0
struct hitbox_list *access_hb_list_index(struct hitbox_list *np, int i)
{
	if (np == NULL)
		return NULL;
	if (i == 0)
		return np;
	return access_hb_list_index(np->next, (i - 1));
}
