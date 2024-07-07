#ifndef SHAPES_H
#define SHAPES_H

struct gl_shape{
	float *vertices;
	int size_v;
	unsigned int *indices;
	int size_i;
};

void destroy_gl_shape(struct gl_shape *);
#endif
