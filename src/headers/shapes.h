#ifndef SHAPES_H
#define SHAPES_H

/* A shape consists of a list of vertices and indices
 * The floats are stored in groups of 2, to represent
 * 2-dimensional coordinates
 * The sizes of each list are stored for reference by other libraries
 * to determine how to parse the arrays
 */
struct gl_shape{
	float *vertices;
	int size_v;
	unsigned int *indices;
	int size_i;
};

// Deallocates memory allocated to a shape by the meshes load_mesh() function
void destroy_gl_shape(struct gl_shape *);
#endif
