#ifndef MESHES_H
#define MESHES_H
struct gl_mesh{
	struct gl_shape *shape;
	struct buffer_t *VBO; 
	struct buffer_t *EBO;
	unsigned int VAO;
	int num_indices;
};

// Take a filename and load vertex data from it
struct gl_shape *load_mesh(const char *);

struct gl_mesh *init_mesh(struct gl_shape *sp);

int build_buffers(struct gl_mesh *);

void destroy_mesh(struct gl_mesh *);

#endif
