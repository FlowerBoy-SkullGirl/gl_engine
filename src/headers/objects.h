#ifndef OBJECTS_H
#define OBJECTS_H

struct game_object{
	// Shape, color, texture
	struct gl_mesh *mesh; 
	struct rgba color;
	unsigned int texture;

	// Hitbox list
	struct hitbox_list *hb_list;

	// Size, location, rotation
	float rotation;
	float pos_x;
	float pos_y;
	float scale_x;
	float scale_y;

	// Mass, physical properties
	struct velocity vel;
	float mass;
	float r_momentum; //rate of conservation of momentum

	// Flags
	int use_tex;
	int use_hitbox;
};

// Create a blank game_object
struct game_object *init_game_object();

// Destroy a game_object
void free_game_object(struct game_object *);

// Modifiers
int set_object_mesh(struct game_object *, struct gl_mesh *);

int set_object_color(struct game_object *, struct rgba);

int set_object_texture(struct game_object *, unsigned int);

int set_object_mass(struct game_object *, float);

// Uniform functions
int set_object_rotation(struct game_object *, float);

int set_object_pos(struct game_object *, float, float);

int set_object_scale(struct game_object *, float, float);

// Hitbox functions
int add_object_hitbox(struct game_object *, struct gl_hitbox *);

// Draw function
void draw_game_object(struct game_object *, unsigned int);

#endif
