#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "headers/rgba.h"

struct rgba RGBA_BG_COLOR;

struct rgba convert_to_rgba(float r, float g, float b, float a)
{
	struct rgba color;
	color.r = r;
	color.g = g;
	color.b = b;
	color.a = a;
	return color;
}

void set_bg_color(struct rgba color)
{
	RGBA_BG_COLOR.r = color.r;
	RGBA_BG_COLOR.g = color.g;
	RGBA_BG_COLOR.b = color.b;
	RGBA_BG_COLOR.a = color.a;
}
