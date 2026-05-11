#ifndef OBJECTS_H
#define OBJECTS_H

enum GL_MeshType {TriangleGLMesh, SquareGLMesh};

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

// Helper function for serializing game objects
// Takes a pointer to a buffer, a pointer to data to be written, an offset from the start of the buffer, 
// the size of the object, and the total length of the buffer
size_t write_to_buffer(char *, char *, size_t, size_t, size_t);

// Serialize into database object
// Allocates memory, so a call to free_serialized_data() must be made afterwards
struct row_object *serialize_game_object(struct game_object *);

// Deserialize game object data from a database row_object
// Allocates memory for a game object, which can be freed using free_game_object()
struct game_object *deserialize_game_object(struct row_object *);

// Free serialized data
struct row_object *free_serialized_data(struct row_object *);

#endif
