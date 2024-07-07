#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "headers/uniforms.h"

void set_uniforms1(float val, unsigned int program, const char *uniform)
{
	int id = glGetUniformLocation(program, uniform);
	glUniform1f(id, val);
}

void set_uniforms2(float x, float y, unsigned int program, const char *uniform)
{
	int id = glGetUniformLocation(program, uniform);
	glUniform2f(id, x, y);
}

void set_uniforms3(float x, float y, float z, unsigned int program, const char *uniform)
{
	int id = glGetUniformLocation(program, uniform);
	glUniform3f(id, x, y, z);
}

void set_uniforms4(float x, float y, float z, float w, unsigned int program, const char *uniform)
{
	int id = glGetUniformLocation(program, uniform);
	glUniform4f(id, x, y, z, w);
}
