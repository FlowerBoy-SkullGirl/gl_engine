#include <iostream>
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "headers/shaders.h"

// C++ complains of literal expansion to char *, so we use const
unsigned int compile_shader(const char *shader_file, GLenum type)
{
	if (shader_file == NULL)
		return -1;
	
	unsigned int shader = glCreateShader(type);

	FILE *fp = fopen(shader_file, "r");
	if (fp == NULL)
		return -1;

	// Determine the size of the shader file
	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// Allocate memory to read the file into a char * array
	char *shader_str = (char *) malloc(fsize);
	if (shader_str == NULL)
		return -1;

	fread(shader_str, 1, fsize, fp);
	shader_str[fsize - 1] = '\0';

	fclose(fp);

	// Compile the shader that was read into a string
	glShaderSource(shader, 1, &shader_str, NULL);
	glCompileShader(shader);

	// Free the memory storing the char * array of the file's contents
	free(shader_str);

	// Report any errors
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success){
		printf("Could not compile the shader at %s\n", shader_file);
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		fprintf(stderr, "%s", infoLog);
		return -1;
	}
	
	return shader;
}

// Wraps the openGL call to link a shader program, but takes care of the work
// of creating the program and attaching the predefined shader to it first
unsigned int link_shaders(unsigned int *shaders, int size)
{
	unsigned int program = glCreateProgram();

	for (int i = 0; i < size; i++){
		glAttachShader(program, shaders[i]);
		glDeleteShader(shaders[i]);
	}

	glLinkProgram(program);
	
	int success;
	char glLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success){
		printf("Could not link the shaders");
		glGetProgramInfoLog(program, 512, NULL, glLog);
		fprintf(stderr, "%s", glLog);
		return -1;
	}

	return program;
}

//Wraps OpenGL's glUseProgram function
void use_shader(unsigned int shader_program)
{
	glUseProgram(shader_program);
}
