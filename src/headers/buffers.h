#ifndef BUFFERS_H
#define BUFFERS_H

struct buffer_t{
	unsigned int id;
	GLenum type;
};

struct buffer_data{
	void *data;
	int size;
	int elements;
	GLenum type;
};

struct buffer_t *init_buffer(GLenum);

int delete_buffer(struct buffer_t *);

// Pointer to data, size of array, num of elements
struct buffer_data pack_data(void *, int, int, GLenum);

// Pointer to buffer, pointer to data, type of data to be passed
int set_buffer(struct buffer_t *, struct buffer_data, GLenum);

int bind_buffer(struct buffer_t *);

unsigned int init_array();

void bind_array(unsigned int);

// Layout, number, GLTYPE, size, offset
void set_array_attributes(int, int, GLenum, int, int, int);

#endif
