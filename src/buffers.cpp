#include <iostream>
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "headers/buffers.h"

// Allocates memory for a buffer_t object, which must be deallocated later with delete_buffer()
struct buffer_t *init_buffer(GLenum type)
{
	struct buffer_t *buf = (buffer_t *)malloc(sizeof(struct buffer_t));;
	buf->type = type;
	glGenBuffers(1, &(buf->id));
	bind_buffer(buf);

	return buf;
}

// Frees the memory allocated by init_buffer(), while also performing OpenGL cleanup
int delete_buffer(struct buffer_t *buf)
{
	if (buf == NULL)
		return -1;
	glDeleteBuffers(1, &(buf->id));
	free(buf);

	return 0;
}

// Takes input to an array of data with a specified size and number of elements and
// formats it into a buffer_data object
struct buffer_data pack_data(void *data, int size, int num, GLenum type)
{
	struct buffer_data b_d;
	b_d.data = data;
	b_d.size = size;
	b_d.elements = num;
	b_d.type = type;
	return b_d;
}

// Wraps OpenGL's glBufferData call
int set_buffer(struct buffer_t *buf, struct buffer_data data, GLenum type)
{
	if (buf == NULL || data.data == NULL)
		return -1;
	glBufferData(buf->type, data.size, data.data, type);
	return 1;
}

// Wraps OpenGL's glBindBuffer call
int bind_buffer(struct buffer_t *buf)
{
	if (buf == NULL)
		return -1;
	glBindBuffer(buf->type, buf->id);
	return 1;
}

// Wraps OpenGL's glBindVertexArray call
void bind_array(unsigned int VAO)
{
	glBindVertexArray(VAO);
}

// Initializes a vertex array object to be used by a mesh object
unsigned int init_array()
{
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	bind_array(VAO);
	return VAO;
}

// Used when building buffers for use with gl_meshes.
void set_array_attributes(int layout, int num, GLenum type, int size, int num_layouts, int offset)
{
	glVertexAttribPointer(layout, num, type, GL_FALSE, num * size * num_layouts, (void *) (offset * size));	
	glEnableVertexAttribArray(layout);
}
