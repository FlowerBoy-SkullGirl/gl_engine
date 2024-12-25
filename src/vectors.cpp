#include <cmath>
#include <iostream>
#include "headers/vectors.h"

// Constants
// All units are metric, kg and m
#define G_CONSTANT 6.674e-11
#define G_PI 3.14159
#define R_EARTH 6.371e6
#define M_EARTH 5.972e24


void rotate2(float *x, float *y, float r)
{
	float s_r = sin(r);
	float c_r = cos(r);
	float new_x = ((*x) * c_r) - ((*y) * s_r);
	float new_y = ((*y) * c_r) + ((*x) * s_r);
	*x = new_x;
	*y = new_y;
}

void translate2(float *x, float *y, float t_x, float t_y)
{
	*x += t_x;
	*y += t_y;
}

void scale2(float *x, float *y, float s_x, float s_y)
{
	*x *= s_x;
	*y *= s_y;
}

void transform2x2(float a, float b, float c, float d, float *x, float *y)
{
	*x = ((*x) * a) + ((*y) * b);
	*y = ((*x) * c) + ((*y) * d);
}

// Find the distance between two points
double distance2(double x1, double y1, double x2, double y2)
{
	// Find the distance between the two masses
	double x_comp = fabs(x1 - x2);
	double y_comp = fabs(y1 - y2);
	
	return sqrt((x_comp * x_comp) + (y_comp * y_comp));
}

// Find the angle between two points in radians
double calc_angle(double x1, double y1, double x2, double y2, double distance)
{
	// Find the delta of x and y between the two objects
	double x_comp = x1 - x2;
	double y_comp = y1 - y2;

	// Define cases where the angle is a multiple of an integer and 1/2 PI
	// Which occurs when there is no change in the x or y dimension
	if (fabs(x_comp/y_comp) < 0.00001f){
		if (y1 < y2)
			return G_PI/2.0f;
		else
			return G_PI * 3.0f/2.0f;
	}
	if (fabs(y_comp/x_comp) < 0.0001f){
		if (x1 < x2)
			return 0.0f;
		else
			return G_PI;
	}
	// The angle can be found by determining the quadrant and the arcsin of the x component
	double angle = acos(x_comp/distance);
	// Bottom quadrants of unit circle
	if (y_comp < 0.0f)
		angle = (G_PI * 2.0f) - angle;
	return angle;
}

// Find the force of gravity between two masses
double calc_force_gravity(double mass1, double mass2, double distance)
{
	// Calculate the force of gravity using the constant G, the distance, and the masses
	return G_CONSTANT * ((mass1 * mass2)/(distance * distance));
}

// Find the delta acceleration due to an application of a force on a mass
double calc_acceleration(double force, double mass)
{
	return force/mass;
}

// Find momentum with velocity and mass
double calc_momentum(struct velocity v, double m)
{
	double x = fabs(v.x);
	double y = fabs(v.y);
	return sqrt((x * x) + (y * y)) * m;
}

// Find a change in velocity due to acceleration
struct velocity find_vel_gravity(double mass1, double x1, double y1, double mass2, double x2, double y2, double d_time)
{
	double distance = calc_distance(x1, y1, x2, y2);
	double angle = calc_angle(x1, y1, x2, y2, distance);
	double force = calc_force_gravity(mass1, mass2, distance);
	double accel = calc_acceleration(force, mass1);

	struct velocity vel;
	// Magnitude * delta_time * scalar
	vel.x = accel * d_time * cos(angle);
	vel.y = accel * d_time * sin(angle);

	return vel;
}

// Use the relative location of object 2 from object 1 to determine which edge of a square it would be approaching from, return the direction of this normal vector 
double find_normal_angle(double x1, double y1, double r1, double x2, double y2)
{
	double angle_approach = calc_angle(x1, y1, x2, y2, distance2(x1, y1, x2, y2));
	
	// The edges of the square can be considered to be quadrants from the center, adjusted for the square's rotation
	if (angle_approach > r1)
		angle_approach -= r1;
	else
		angle_approach += ((2.0 * G_PI) - r1);
	
	// If the previous operation causes the angle to exceed G_PI / 2.0f, reduce the angle to our defined range
	angle_approach = fmodl(angle_approach, (G_PI * 2.0f));
	// If the angle is below 0, adjust similarly
	if (angle_approach < 0)
		angle_approach = (G_PI * 2.0f) - angle_approach;

	// Positive y, neutral x quadrant
	if (angle_approach >= (G_PI / 4.0f) && angle_approach <= (3.0f * G_PI / 4.0f))
		return (G_PI / 2.0f) + r1;
	
	// Neutral y, negative x quadrant
	if (angle_approach > (3.0f * G_PI / 4.0f) && angle_approach < (5.0f * G_PI / 4.0f))
		return G_PI + r1;
	
	// Negative y, neutral x quadrant
	if (angle_approach >= (5.0f * G_PI / 4.0f) && angle_approach <= (7.0f * G_PI / 4.0f))
		return (3.0f * G_PI / 2.0f) + r1;
	
	// Neutral y, positive x quadrant
	if (angle_approach > (7.0f * G_PI / 4.0f) || angle_approach < (G_PI / 4.0f))
		return r1; // The same as saying 0 + r1 or G_PI * 2 + r1
	
	// Default case that should not be reached
	return -1.0f;
}

// The velocity of the larger object will be affected less by the total change in momentum than the smaller object
double find_normal_magnitude(double m1, double m2)
{
	double magnitude = m2 / m1;
	if (magnitude > 1.0f)
		return 1.0f;
	return magnitude / (m1/m2);
}

struct velocity find_v_after_collision(double mag_N, double r, struct velocity v)
{
	// Find the magnitude by inner product of component vectors
	double mag_v = distance2(v.x, 0.0f, 0.0f, v.y);
	
	struct new_v;
	new_v.x = cos(r) * mag_N * mag_v;
	new_v.y = sin(r) * mag_N * mag_v;

	return new_v;
}

double max_dimension(double sx, double sy)
{
	if (sx > sy)
		return sx;
	return sy;
}

double min_dimension(double sx, double sy)
{
	if (sx < sy)
		return sx;
	return sy;
}
