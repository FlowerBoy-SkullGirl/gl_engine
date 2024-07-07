#ifndef SHADERS_H
#define SHADERS_H

// Shader files to be loaded
#define V_SHADER_FILE "shader/vertex.glsl"
#define F_SHADER_FILE "shader/fragment.glsl"

// Take a shader string and compile it into a shader
// C ++ complains of literal expansion to char *, so we use const
unsigned int compile_shader(const char *, GLenum);

// Take a list of shaders and link them to a shader program, return the program ID
unsigned int link_shaders(unsigned int *, int);

// Wrapper for glUseProgram
void use_shader(unsigned int);

#endif
