#ifndef TEXTURES_H
#define TEXTURES_H

// load a texture from a file
unsigned int load_texture(const char *);

// Wraps the OpenGL glBindTexture function
void bind_texture(unsigned int);
#endif
