#ifndef RGBA_H
#define RGBA_H

// A 4 dimensional vector used to represent colors with float values
struct rgba{
	float r;
	float g;
	float b;
	float a;
};

// Packs 4 float values into an rgba struct
struct rgba convert_to_rgba(float, float, float, float);

// Sets the rgba color to be used for the window background
void set_bg_color(struct rgba);

#endif
