#ifndef UNIFORMS_H
#define UNIFORMS_H

/* This library is used to pass values from the main program
 * to the GPU shader programs through values called 'uniforms'
 */

// Value, program, name
void set_uniforms1(float, unsigned int, const char *);

// Value, value, program, name
void set_uniforms2(float, float, unsigned int, const char *);

void set_uniforms3(float, float, float, unsigned int, const char *);

void set_uniforms4(float, float, float, float, unsigned int, const char *);

void set_uniform_int(int, unsigned int, const char *);

#endif
