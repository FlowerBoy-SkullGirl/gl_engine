#include <iostream>

//#include <GL/glew.h>
//#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "headers/database.h"
#include "headers/shapes.h"
#include "headers/shaders.h"
#include "headers/buffers.h"
#include "headers/uniforms.h"
#include "headers/rgba.h"
#include "headers/meshes.h"
#include "headers/textures.h"
#include "headers/vectors.h"
#include "headers/hitbox.h"
#include "headers/hitbox_list.h"
#include "headers/objects.h"
#include "headers/camera.h"
#include "headers/object_list.h"
#include "headers/collision.h"
#include "headers/keybinds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "headers/stb_image.h"

#define V_PI 3.1415

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_NAME_MAX 256

#define WORLD_SCALE 0.025
#define BASE_VEL 3.0
#define RATE_ACCELERATION 0.7
//Allow debugging from attached GDB
#include <sys/prctl.h>
void allow_debug()
{       
	prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
}

// Externs
extern struct rgba RGBA_BG_COLOR;
extern float g_world_scale;
extern float g_cam_x;
extern float g_cam_y;
extern float g_cam_max_speed;
extern float g_cam_tether_distance;
extern void (*keybind_actions[MAX_SCANCODES])(void);
extern int keybind_list[MAX_SCANCODES];

// Local globals
float g_time;
float g_delta_t;

struct game_object *player_object;

GLFWwindow* main_game_window;

int g_debugging = 0;

// Callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// Use the player object's velocity to determine it's position each screen draw
void move_player_object()
{
	set_object_pos(player_object, player_object->pos_x + ((player_object->vel).x * g_delta_t), player_object->pos_y + ((player_object->vel).y * g_delta_t));
}

void processInput(GLFWwindow *window)
{
    process_keybinds();
    move_player_object();
}

void exit_window()
{
        glfwSetWindowShouldClose(main_game_window, true);
}

/*
 * Input functions change the player object's velocity based on the max velocity, the rate of acceleration, and the change in time since
 * last screen draw. The min value is taken between the calculated new velocity and the max velocity.
 * Actual changes to the player object position are done each screen draw by "move_player_object().
 */
void player_up()
{
	struct velocity vel;
	vel.y = std::min((BASE_VEL * RATE_ACCELERATION * g_delta_t) + (player_object->vel).y, BASE_VEL);
	(player_object->vel).y = vel.y;
	set_object_rotation(player_object, 0.0f);
}

void player_down()
{
	struct velocity vel;
	vel.y = std::min((BASE_VEL * RATE_ACCELERATION * g_delta_t) + fabs((player_object->vel).y), BASE_VEL);
	vel.y *= -1.0f;
	(player_object->vel).y = vel.y;
	set_object_rotation(player_object, V_PI);
}

void player_left()
{
	struct velocity vel;
	vel.x = std::min((BASE_VEL * RATE_ACCELERATION * g_delta_t) + fabs((player_object->vel).x), BASE_VEL);
	vel.x *= -1.0f;
	(player_object->vel).x = vel.x;
	set_object_rotation(player_object, (3.0f * V_PI/2.0f));
}

void player_right()
{
	struct velocity vel;
	vel.x = std::min((BASE_VEL * RATE_ACCELERATION * g_delta_t) + (player_object->vel).x, BASE_VEL);
	(player_object->vel).x = vel.x;
	set_object_rotation(player_object, (V_PI/2.0f));
}

void glfw_error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}

int main()
{
	// Enable debugging throughout program
	g_debugging = 1;

	const char window_name[WINDOW_NAME_MAX] = "Square";

	if (g_debugging)
		allow_debug();

	/* CREATE WINDOW SECTION START
	 * Use glfw to create the window in the operating system which OpenGL will render to 
	 */
	if(!glfwInit())
	{
		std::cout << "Failed glfw init" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSetErrorCallback(glfw_error_callback);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, window_name, NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	main_game_window = window;

	glfwMakeContextCurrent(window);

	// Glad has replaced Glew as the library used to bind OpenGL calls
	// See commit a2fa2b8fac601c16c9487ad0666218deb9db45c6
	gladLoadGL(glfwGetProcAddress);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	/* LOAD SHAPES FOR BUFFERS */

	struct gl_shape *square_shape = load_mesh("meshes/square.glMesh");	
	struct gl_shape *triangle_shape = load_mesh("meshes/triangle.glMesh");	

	struct gl_mesh *square_mesh = init_mesh(square_shape);
	struct gl_mesh *triangle_mesh = init_mesh(triangle_shape);

	build_buffers(square_mesh);
	build_buffers(triangle_mesh);

	bind_array(0);

	/* LOAD MESH FOR HITBOXES */
	struct gl_shape *square_hitbox_shape = load_mesh("meshes/square_hitbox.glMesh");
	struct gl_mesh *square_hitbox_mesh = init_mesh(square_hitbox_shape);

	/* LOAD TEXTURES */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stbi_set_flip_vertically_on_load(true);
	unsigned int cloud_tex = load_texture("textures/cloud.png");

	/* CREATE HITBOXES */
	struct gl_hitbox *square_hitbox = create_hitbox(square_hitbox_mesh);
	struct gl_hitbox *tall_square_hitbox = create_hitbox(square_hitbox_mesh);
	tall_square_hitbox->scale_y = 4.0f;

	/* CREATE OBJECTS WITH BUFFERS 
	 * Objects are also appended to a specific object list,
	 * objects not in these lists are not rendered in the main loop
	 * or considered for collision checks
	 */
	struct object_list *bg_objects = create_object_list_node();
	struct object_list *mid_objects = create_object_list_node();
	struct object_list *fg_objects = create_object_list_node();

	struct game_object *tall_square = init_game_object();
	set_object_mesh(tall_square, square_mesh);
	set_object_rotation(tall_square, (V_PI/6.0f));
	set_object_pos(tall_square, 2.0f, -4.0f); 
	set_object_scale(tall_square, 1.0f, 4.0f); 
	add_object_hitbox(tall_square, tall_square_hitbox);
	append_object(mid_objects, tall_square);

	struct game_object *spinning_square = init_game_object();
	set_object_mesh(spinning_square, square_mesh);
	set_object_scale(spinning_square, 3.0f, 3.0f);
	set_object_color(spinning_square, convert_to_rgba(1.0f, 0.0f, 0.0f, 1.0f));
	append_object(mid_objects, spinning_square);

	struct game_object *weird_triangle = init_game_object();
	set_object_mesh(weird_triangle, triangle_mesh);
	set_object_rotation(weird_triangle, (V_PI/4.0f));
	set_object_pos(weird_triangle, 10.0f, 14.0f);
	set_object_scale(weird_triangle, 2.0f, 2.0f);
	set_object_color(weird_triangle, convert_to_rgba(0.0f, 1.0f, 0.0f, 1.0f));
	append_object(mid_objects, weird_triangle);

	struct game_object *cloud1 = init_game_object();
	set_object_mesh(cloud1, square_mesh);
	set_object_scale(cloud1, 4.0f, 2.0f);
	set_object_texture(cloud1, cloud_tex);
	append_object(fg_objects, cloud1);

	struct game_object *cloud2 = init_game_object();
	set_object_mesh(cloud2, square_mesh);
	set_object_scale(cloud2, 4.0f, 2.0f);
	set_object_texture(cloud2, cloud_tex);
	append_object(bg_objects, cloud2);

	// Player object
	player_object = init_game_object();
	set_object_mesh(player_object, triangle_mesh);
	set_object_scale(player_object, 3.0f, 3.0f);
	set_object_pos(player_object, 0.0f, 0.0f);
	set_object_color(player_object, convert_to_rgba(0.0f, 0.0f, 1.0f, 0.8f));

	add_object_hitbox(player_object, square_hitbox);
	append_object(mid_objects, player_object);


	//Initialize VECTOR MODE
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* LOAD SHADERS START */
	unsigned int vertexShader = compile_shader(V_SHADER_FILE, GL_VERTEX_SHADER);

	unsigned int fragmentShader = compile_shader(F_SHADER_FILE, GL_FRAGMENT_SHADER);

	unsigned int shaders[] = {vertexShader, fragmentShader};

	unsigned int shader1 = link_shaders(shaders, sizeof(shaders)/sizeof(shaders[0]));

	use_shader(shader1);	


	/* Debugging information */
	if (g_debugging){
		// Get errors from OpenGL before the main loop
		int glStatus = (int) glGetError();
		fprintf(stdout, "Preloop %d\n", glStatus);
		// Enable the fragment shader debugging uniform
		set_uniform_int(g_debugging, shader1, "debuggingEnabled");
	}
	
	/* Pre-screen configuration */
	// Color selection for background of window
	struct rgba bg_color = convert_to_rgba(0.0f, 0.1f, 0.1f, 1.0f);
	set_bg_color(bg_color);

	// Scale to transform from world to screen space
	set_world_scale(WORLD_SCALE);
	// Camera posistion determines the origin of view space
	set_cam_pos(0.0f, 0.0f);
	// Determine how the camera will move in the main loop
	//set_camera_movement_type(glCamFixed, player_object);
	set_camera_movement_type(glCamTethered, player_object);
	set_cam_tether_distance(10.0f);

	// Bind keys to action functions using GLFW to capture input
	register_key_action_pair(GLFW_KEY_F, &player_up);
	register_key_action_pair(GLFW_KEY_S, &player_down);
	register_key_action_pair(GLFW_KEY_R, &player_left);
	register_key_action_pair(GLFW_KEY_T, &player_right);
	register_key_action_pair(GLFW_KEY_ESCAPE, &exit_window);

	/* MAIN LOOP START
	 * Physics and rendering both take place within this function
	 * input is captured before the frame is drawn
	 */
	while(!glfwWindowShouldClose(window))
	{
		// Process delta time
		float time = glfwGetTime();
		g_delta_t = time - g_time;
		g_time = time;
		
		// Process input
		processInput(window);

		// Clear the screen
		glClearColor(RGBA_BG_COLOR.r, RGBA_BG_COLOR.g, RGBA_BG_COLOR.b, RGBA_BG_COLOR.a);
		glClear(GL_COLOR_BUFFER_BIT);

		// Use the vertex shader
		use_shader(shader1);


		// Set uniform values
		// Camera position and scale
		move_camera();
		update_camera(shader1);
		set_uniforms2(g_world_scale, g_world_scale, shader1, "screen_scalar");

		// Prepare spinning square
		float rotation_val = glfwGetTime();
		set_object_rotation(spinning_square, rotation_val);
		set_object_pos(spinning_square, -7.0f, -14.0f);

		// Draw a cloud
		float cloud_x = (1.0 / WORLD_SCALE) * sin(glfwGetTime() / 30.0f);
		float cloud_y = 0.0f;
		set_object_pos(cloud1, cloud_x, cloud_y);

		// Draw more clouds
		cloud_x += 3.0f;
		cloud_x *= 1.3f;
		cloud_y -= 4.0f;
		set_object_pos(cloud2, cloud_x, cloud_y);

		// Draw all background objects
		for (int i = 0; access_go_list_index(bg_objects, i) != NULL; i++){
			draw_game_object((access_go_list_index(bg_objects, i))->op, shader1);
		}

		// Draw all middling objects
		for (int i = 0; access_go_list_index(mid_objects, i) != NULL; i++){
			draw_game_object((access_go_list_index(mid_objects, i))->op, shader1);
		}

		// Draw all foreground objects
		for (int i = 0; access_go_list_index(fg_objects, i) != NULL; i++){
			draw_game_object((access_go_list_index(fg_objects, i))->op, shader1);
		}

		// Determine collisions
		if (check_collision_objects(player_object, tall_square)){
			// Change object color if debugging is enabled
			if (g_debugging)
				set_object_color(player_object, convert_to_rgba(0.7f, 0.0f, 0.3f, 0.8f));
		}
		else{
			set_object_color(player_object, convert_to_rgba(0.0f, 0.0f, 1.0f, 0.8f));
		}


		glfwSwapBuffers(window);
		glfwPollEvents();    
		bind_array(0);
	}


	// Testing serialization
	struct row_object *ro = serialize_game_object(player_object);
	char *serial_string = serial_to_string(ro);
	printf("Serial string: %s\n", serial_string);

	char *data_type_list_str = type_list_to_string(ro->data_type_list, ro->column_count);

	free_serialized_data(ro);
	ro = NULL;

	ro = string_to_serial(serial_string, data_type_list_str);

	free(serial_string);
	serial_string = NULL;

	free(data_type_list_str);
	data_type_list_str = NULL;

	serial_string = serial_to_string(ro);
	printf("Serial string double converted: %s\n", serial_string);


	// Testing database
	struct gl_db *db = create_database("database/game.db");
	if (db == NULL)
		db = open_database("database/game.db");
	if (find_table_by_id(1, db) == 0){
		add_table_to_db("GameObjects", db);
	}
	if (find_table_by_id(2, db) == 0){
		add_table_to_db("SecondTable", db);
	}
	off_t table1_pos = find_table_by_id(1,db);
	off_t table2_pos = find_table_by_id(2,db);

	printf("Table positions: %ld, %ld\n", table1_pos, table2_pos);

	struct table_metadata table1_md = get_table_metadata(1, db);

	printf("Table metadata: %d, %d, %d, %d\n", table1_md.id, table1_md.num_rows, table1_md.newest_row_id, table1_md.num_cols);

	struct table_metadata table2_md = get_table_metadata(2, db);

	printf("Table2 metadata: %d, %d, %d, %d\n", table2_md.id, table2_md.num_rows, table2_md.newest_row_id, table2_md.num_cols);

	write_column_data_types_to_table(ro->data_type_list, ro->column_count, 1, db);
	// Update metadata after write
	table1_md = get_table_metadata(1, db);

	enum DB_TYPES *types_table1 = get_data_types_list(1, db);
	char *types_table1_string = type_list_to_string(types_table1, table1_md.num_cols - 1);

	printf("Get types list: %s\n", types_table1_string);

	add_row_to_table(ro, 1, db);
	int row1_pos = find_row_by_id(1, 1, db);
	printf("Row 1 pos: %d\n", row1_pos);

	struct row_object *ro2 = get_row_data(2, 1, db);
	char *ro2_data = serial_to_string(ro2);

	printf("Row 2 data: %s\n", ro2_data);

	remove_row_from_table(1, 1, db);

	free(types_table1);
	free(types_table1_string);
	free_serialized_data(ro);
	free_serialized_data(ro2);
	free(serial_string);
	free(ro2_data);
	close_database(db);

	/* CLEAN UP */
	glDeleteProgram(shader1);

	destroy_mesh(square_mesh);
	destroy_mesh(triangle_mesh);

	destroy_object_list(bg_objects);
	destroy_object_list(mid_objects);
	destroy_object_list(fg_objects);

	free_collision_memory();

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  
