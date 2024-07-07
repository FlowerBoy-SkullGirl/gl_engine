#ifndef RGBA_H
#define RGBA_H

struct rgba{
	float r;
	float g;
	float b;
	float a;
};

struct rgba convert_to_rgba(float, float, float, float);

void set_bg_color(struct rgba);

#endif
