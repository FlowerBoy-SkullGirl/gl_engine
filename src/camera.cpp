#include "headers/camera.h"
#include "headers/uniforms.h"

// Global values
float g_world_scale;
float g_cam_x;
float g_cam_y;

// Sets the scale for transformations between world space and screen space coordinates
void set_world_scale(float s)
{
	g_world_scale = s;
}

// Sets the position of view space origins
void set_cam_pos(float x, float y)
{
	g_cam_x = x;
	g_cam_y = y;
}

// Syncs the camera object position with the values in the cameraPos uniform in the vertex shader
void update_camera(unsigned int shader)
{
	set_uniforms2(g_cam_x, g_cam_y, shader, "cameraPos");
}
