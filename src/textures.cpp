#include <iostream>
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "headers/textures.h"
#include "headers/stb_image.h"

// load a texture from a file, using stbi to load an RGBA image
unsigned int load_texture(const char *filen)
{
	// Create the texture
	unsigned int texID = 0;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load the image
	int x, y, ch;
	unsigned char *image = stbi_load(filen, &x, &y, &ch, 0); 

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image);
	
	return texID;
}

// Wraps the OpenGL glBindTexture function
void bind_texture(unsigned int id)
{
	glBindTexture(GL_TEXTURE_2D, id);
}
