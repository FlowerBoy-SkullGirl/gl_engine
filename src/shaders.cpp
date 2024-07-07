#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "headers/shaders.h"

// C ++ complains of literal expansion to char *, so we use const
unsigned int compile_shader(const char *shader_file, GLenum type)
{
	if (shader_file == NULL)
		return -1;
	
	unsigned int shader = glCreateShader(type);

	FILE *fp = fopen(shader_file, "r");
	if (fp == NULL)
		return -1;

	fseek(fp, 0, SEEK_END);
	long fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *shader_str = (char *) malloc(fsize);
	if (shader_str == NULL)
		return -1;

	fread(shader_str, 1, fsize, fp);
	shader_str[fsize - 1] = '\0';

	fclose(fp);

	glShaderSource(shader, 1, &shader_str, NULL);
	glCompileShader(shader);

	free(shader_str);

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


void use_shader(unsigned int shader_program)
{
	glUseProgram(shader_program);
}
