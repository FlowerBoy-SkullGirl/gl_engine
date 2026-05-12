#ifndef CAMERA_H
#define CAMERA_H

// An enumerator used to clarify what type of movement the camera should follow
// glCamStatic stays in one location in world space, glCamFixed is 'fixed' to the same coordinates as
// a focus object, and glCamTethered is 'tethered' at a distance to a focus object
enum glCamMovementType {glCamStatic, glCamFixed, glCamTethered};

// A struct to hold global values for the camera
struct gl_camera {
	float x;
	float y;
	float max_speed;
	struct velocity vel;
	float tether_distance;
	enum glCamMovementType movement_mode;
	struct game_object *focus;
};

// Initialize default values for the camera struct
void init_camera();

// Sets the scale for transformations between world space and screen space coordinates
void set_world_scale(float);

// Sets the position of view space origins
void set_cam_pos(float, float);

// Sets the camera movement type based on a glCamMovementType enum selection and a focus object
// If glCamStatic is the enum type, NULL should be passed as the focus object
void set_camera_movement_type(enum glCamMovementType, struct game_object *);

// Set the distance at which the camera will follow the tethered focus
void set_cam_tether_distance(float);

// Set the maximum speed that the camera will move at when not past the tether length
void set_cam_max_speed(float);

// Calculate the tethered movement of the camera and return a velocity struct
struct velocity calculate_tether();

// Sets the camera based on the selected movement type
void move_camera();

// Syncs the camera object position with the values in the cameraPos uniform in the vertex shader
void update_camera(unsigned int);

#endif
