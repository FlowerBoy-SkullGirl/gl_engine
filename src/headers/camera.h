#ifndef CAMERA_H
#define CAMERA_H

// Sets the scale for transformations between world space and screen space coordinates
void set_world_scale(float);
// Sets the position of view space origins
void set_cam_pos(float, float);
// Syncs the camera object position with the values in the cameraPos uniform in the vertex shader
void update_camera(unsigned int);

#endif
