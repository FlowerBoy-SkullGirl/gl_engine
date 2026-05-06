#ifndef MESHES_H
#define MESHES_H

/* A mesh contains a shape, which stores information about the vertices that make up the mesh
 * One OpenGL VBO, VAO, and EBO, which are used to pass to OpenGL functions that will draw the mesh
 * And a number of total indices that are required to draw the mesh with triangles
 */
struct gl_mesh{
	struct gl_shape *shape;
	struct buffer_t *VBO; 
	struct buffer_t *EBO;
	unsigned int VAO;
	int num_indices;
};

// Take a filename and load vertex data from it
struct gl_shape *load_mesh(const char *);

// Takes a pointer to a valid gl_shape and allocates memory for a mesh that uses that shape
struct gl_mesh *init_mesh(struct gl_shape *sp);

// Abstracts the creation of OpenGL buffer objects for a given mesh, returns 0 on success
int build_buffers(struct gl_mesh *);

// Performs cleanup and frees the memory of buffers, shapes, and the mesh itself
void destroy_mesh(struct gl_mesh *);

#endif
