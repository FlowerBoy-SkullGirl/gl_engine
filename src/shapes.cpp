#include <stdlib.h>
#include "headers/shapes.h"

// Frees memory of member arrays, then frees the memory allocated to the struct *argument
void destroy_gl_shape(struct gl_shape *sp)
{
	if (sp == NULL)
		return;
	if (sp->vertices != NULL)
		free(sp->vertices);
	if (sp->indices != NULL)
		free(sp->indices);

	free(sp);
}
