#ifndef VECTORS_H
#define VECTORS_H

struct velocity{
	double x;
	double y;
};

void rotate2(float *, float *, float);

void translate2(float *, float *, float, float);

void scale2(float *, float *, float, float);

void transform2x2(float, float, float, float, float *, float *);

double distance2(double, double, double, double);

// x1, y1, x2, y2, distance
double calc_angle(double, double, double, double, double);

double calc_force_gravity(double, double, double);

double calc_acceleration(double, double);

double calc_momentum(struct velocity, double);

// m1, x1, y1, m2, x2, y2, delta_time
struct velocity find_vel_gravity(double, double, double, double, double, double, double);

// x1, y1, r1, x2, y2
double find_normal_angle(double, double, double, double, double);

// Mass 1, mass 2
double find_normal_magnitude(double, double);

// Normalized magnitude of normal force, angle of new trajectory, and original velocity magnitude
struct velocity find_v_after_collision(double, double, struct velocity v);

double max_dimension(double, double);

double min_dimension(double, double);

#endif
