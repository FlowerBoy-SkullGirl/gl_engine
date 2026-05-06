#ifndef BUFFERS_H
#define BUFFERS_H

struct buffer_t{
	unsigned int id;
	GLenum type;
};

// A void * is selected to hold the data because several types of data may be stored in this struct
// The type of the data will be inferred from GLenum type and the size listed in bytes
struct buffer_data{
	void *data;
	int size;
	int elements;
	GLenum type;
};

// Allocates memory for a buffer_t object, which must be deallocated later with delete_buffer()
struct buffer_t *init_buffer(GLenum);

// Frees the memory allocated by init_buffer(), while also performing OpenGL cleanup
int delete_buffer(struct buffer_t *);

// Pointer to data, size of array, num of elements
struct buffer_data pack_data(void *, int, int, GLenum);

// Pointer to buffer, pointer to data, type of data to be passed
int set_buffer(struct buffer_t *, struct buffer_data, GLenum);

// Wraps OpenGL's glBufferData call
int bind_buffer(struct buffer_t *);

// Initializes a vertex array object to be used by a mesh object
unsigned int init_array();

// Wraps OpenGL's glBindVertexArray call
void bind_array(unsigned int);

// Layout, number, GLTYPE, size, offset
void set_array_attributes(int, int, GLenum, int, int, int);

#endif
