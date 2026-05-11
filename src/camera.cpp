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

// Global values
float g_world_scale;
float g_cam_x = DEFAULT_GL_CAM_POS;
float g_cam_y = DEFAULT_GL_CAM_POS;
float g_cam_max_speed = DEFAULT_GL_CAM_SPEED;
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
	struct velocity cam_vel;
	cam_vel.x = 0.0;
	cam_vel.y = 0.0;

	if (g_cam_focus == NULL)
		return cam_vel; 

	double d = distance2(g_cam_x, g_cam_y, g_cam_focus->pos_x, g_cam_focus->pos_y);
	if (d >= g_cam_tether_distance){
		//TODO: Implement algorithm that finds the angle between camera and focus and moves
		// Camera towards focus object at a rate that will preserve distance
		set_cam_pos(g_cam_focus->pos_x, g_cam_focus->pos_y);
		cam_vel.x = 0.0;
		cam_vel.y = 0.0;
	}else{
		cam_vel.x = 0.0;
		cam_vel.y = 0.0;
	}
	return cam_vel;
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
			struct velocity cam_vel;
			cam_vel = calculate_tether();
			set_cam_pos(g_cam_x + (cam_vel.x * g_delta_t), g_cam_y + (cam_vel.y * g_delta_t));
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
