#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "headers/buffers.h"

struct buffer_t *init_buffer(GLenum type)
{
	struct buffer_t *buf = (buffer_t *)malloc(sizeof(struct buffer_t));;
	buf->type = type;
	glGenBuffers(1, &(buf->id));
	bind_buffer(buf);

	return buf;
}

int delete_buffer(struct buffer_t *buf)
{
	if (buf == NULL)
		return -1;
	glDeleteBuffers(1, &(buf->id));
	free(buf);

	return 0;
}

struct buffer_data pack_data(void *data, int size, int num, GLenum type)
{
	struct buffer_data b_d;
	b_d.data = data;
	b_d.size = size;
	b_d.elements = num;
	b_d.type = type;
	return b_d;
}

int set_buffer(struct buffer_t *buf, struct buffer_data data, GLenum type)
{
	if (buf == NULL || data.data == NULL)
		return -1;
	glBufferData(buf->type, data.size, data.data, type);
	return 1;
}

int bind_buffer(struct buffer_t *buf)
{
	if (buf == NULL)
		return -1;
	glBindBuffer(buf->type, buf->id);
	return 1;
}

void bind_array(unsigned int VAO)
{
	glBindVertexArray(VAO);
}

unsigned int init_array()
{
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	bind_array(VAO);
	return VAO;
}

void set_array_attributes(int layout, int num, GLenum type, int size)
{
	glVertexAttribPointer(layout, num, type, GL_FALSE, num * size, (void *) 0);	
	glEnableVertexAttribArray(layout);
}
