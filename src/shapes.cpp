#include <stdlib.h>
#include "headers/shapes.h"

void destroy_gl_shape(struct gl_shape *sp)
{
	if (sp == NULL)
		return;
	if (sp->vertices != NULL)
		free(sp->vertices);
	if (sp->indices != NULL)
		free(sp->vertices);

	free(sp);
}
