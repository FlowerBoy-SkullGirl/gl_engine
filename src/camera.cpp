#include <cmath> // for fabs - float absolute values
#include "headers/camera.h"
#include "headers/uniforms.h" // Set camera uniforms
#include "headers/vectors.h" // Use velocity struct
#include "headers/rgba.h"
#include "headers/objects.h" // Use game_object struct

#define DEFAULT_GL_CAM_SPEED 1.0f
#define DEFAULT_GL_CAM_TETHER_D 0.0f
#define DEFAULT_GL_CAM_POS 0.0f
#define DEFAULT_GL_CAM_FOCUS NULL
#define RATE_CAMERA_SLOW 0.3
#define OFFSET_ERROR 0.999

#include <iostream> // For debugging only

// Global values
float g_world_scale;
float g_cam_x = DEFAULT_GL_CAM_POS;
float g_cam_y = DEFAULT_GL_CAM_POS;
float g_cam_max_speed = DEFAULT_GL_CAM_SPEED;
struct velocity g_cam_vel;
// The maximum distance that the camera will travel from the focus object if movement mode is set to 'tethered'
float g_cam_tether_distance = DEFAULT_GL_CAM_TETHER_D;
enum glCamMovementType g_cam_movement_mode;
struct game_object *g_cam_focus = DEFAULT_GL_CAM_FOCUS;

// Extern value from main.cpp that represents time since last frame
extern float g_delta_t;

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

// Sets the camera movement type globals to the given arguments
void set_camera_movement_type(enum glCamMovementType mode, struct game_object *focus)
{
	g_cam_movement_mode = mode;
	g_cam_focus = focus;
	g_cam_vel.x = 0.0;
	g_cam_vel.y = 0.0;
}

// Sets the camera tether distance
void set_cam_tether_distance(float d)
{
	g_cam_tether_distance = fabs(d);
}

// Set the camera max speed
void set_cam_max_speed(float s)
{
	g_cam_max_speed = fabs(s);
}

// Calculate the tethered movement of the camera
struct velocity calculate_tether()
{
	if (g_cam_focus == NULL)
		return g_cam_vel; 

	double d = distance2(g_cam_x, g_cam_y, g_cam_focus->pos_x, g_cam_focus->pos_y);
	if (d >= g_cam_tether_distance){
		// Find angle between camera and focus object
		double a = calc_angle(g_cam_focus->pos_x, g_cam_focus->pos_y, g_cam_x, g_cam_y, d);
		// Maintain the same angle, but change the magnitude to exactly g_cam_tether_distance
		float offset_x = cos(a) * g_cam_tether_distance;
		float offset_y = sin(a) * g_cam_tether_distance;
		set_cam_pos(g_cam_focus->pos_x - offset_x, g_cam_focus->pos_y - offset_y);
		// Set the camera velocity to the focus object's velocity, as if it's being 'dragged'
		g_cam_vel.x = (g_cam_focus->vel).x;
		g_cam_vel.y = (g_cam_focus->vel).y;
	}else{
		// Continue moving, but slow down as the camera continues to approach the object
		g_cam_vel.x -= g_cam_vel.x * RATE_CAMERA_SLOW * g_delta_t;
		g_cam_vel.y -= g_cam_vel.y * RATE_CAMERA_SLOW * g_delta_t;
	}
	return g_cam_vel;
}

// Sets the camera position based on the selected movement type
void move_camera()
{
	if (g_cam_focus == NULL)
		return;
	switch (g_cam_movement_mode){
		case glCamStatic:
			break;
		case glCamFixed:
			// Set the camera position to the same position as the focus object
			set_cam_pos(g_cam_focus->pos_x, g_cam_focus->pos_y);
			break;
		case glCamTethered:
			g_cam_vel = calculate_tether();
			set_cam_pos(g_cam_x + (g_cam_vel.x * g_delta_t), g_cam_y + (g_cam_vel.y * g_delta_t));
			break;
		default:
			break;
	}
}

// Syncs the camera object position with the values in the cameraPos uniform in the vertex shader
void update_camera(unsigned int shader)
{
	set_uniforms2(g_cam_x, g_cam_y, shader, "cameraPos");
}
