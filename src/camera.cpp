#include "headers/camera.h"
#include "headers/uniforms.h"

float g_world_scale;
float g_cam_x;
float g_cam_y;

void set_world_scale(float s)
{
	g_world_scale = s;
}

void set_cam_pos(float x, float y)
{
	g_cam_x = x;
	g_cam_y = y;
}

void update_camera(unsigned int shader)
{
	set_uniforms2(g_cam_x, g_cam_y, shader, "cameraPos");
}
