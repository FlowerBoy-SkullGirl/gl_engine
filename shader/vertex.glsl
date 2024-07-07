#version 330 core
layout (location = 0) in vec2 local_coords;

uniform float rotationRad;
uniform vec2 scalars;
uniform vec2 translation;
uniform vec2 screen_scalar;


vec2 rotate(inout vec2 coords)
{
	float c_r = cos(rotationRad);
	float s_r = sin(rotationRad);
	mat2 r_m = mat2(c_r, -s_r, s_r, c_r);

	return r_m * coords;
}

vec2 scale(inout vec2 coords)
{
	return coords * scalars;
}

vec2 translate(inout vec2 coords)
{
	return coords + translation;
}

vec2 scale_to_screen(inout vec2 coords)
{
	return coords * screen_scalar;
}

vec2 transformations(in vec2 coords)
{
	vec2 coords_t = coords;
	return scale(translate(rotate(coords_t)));	
}

void main()
{
	vec2 worldCoords = vec2(transformations(local_coords));
	vec2 screenCoords = vec2(scale_to_screen(worldCoords));
	gl_Position = vec4(screenCoords.x, screenCoords.y, 0.0, 1.0);
}
