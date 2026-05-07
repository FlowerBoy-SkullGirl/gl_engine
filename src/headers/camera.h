#ifndef CAMERA_H
#define CAMERA_H

enum glCamMovementType {glCamStatic, glCamFixed, glCamTethered};

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
