#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

#define WORLD_SCALE 0.025
#define BASE_VEL 3.0
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
extern int keybind_up;
extern int keybind_down;
extern int keybind_left;
extern int keybind_right;

// Local globals
float g_time;
float g_delta_t;

struct game_object *player_object;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, keybind_up) == GLFW_PRESS){
	set_object_pos(player_object, player_object->pos_x, player_object->pos_y + (BASE_VEL * g_delta_t));
	set_object_rotation(player_object, 0.0f);
    }
    if(glfwGetKey(window, keybind_down) == GLFW_PRESS){
	set_object_pos(player_object, player_object->pos_x, player_object->pos_y - (BASE_VEL * g_delta_t));
	set_object_rotation(player_object, V_PI);
    }
    if(glfwGetKey(window, keybind_left) == GLFW_PRESS){
	set_object_pos(player_object, player_object->pos_x - (BASE_VEL * g_delta_t), player_object->pos_y);
	set_object_rotation(player_object, (3.0f * V_PI/2.0f));
    }
    if(glfwGetKey(window, keybind_right) == GLFW_PRESS){
	set_object_pos(player_object, player_object->pos_x + (BASE_VEL * g_delta_t), player_object->pos_y);
	set_object_rotation(player_object, (V_PI/2.0f));
    }
	  
}

int main()
{
	allow_debug();
/* CREATE WINDOW SECTION START */
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Square", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed glew" << std::endl;
		return -2;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

/* CREATE WINDOW SECTION DONE */

/* LOAD SHAPES FOR BUFFERS */

	struct gl_shape *square_shape = load_mesh("meshes/square.glMesh");	
	struct gl_shape *triangle_shape = load_mesh("meshes/triangle.glMesh");	

	struct gl_mesh *square_mesh = init_mesh(square_shape);
	struct gl_mesh *triangle_mesh = init_mesh(triangle_shape);

	build_buffers(square_mesh);
	build_buffers(triangle_mesh);

	bind_array(0);
/* END LOAD SHAPES */

/* LOAD MESH FOR HITBOXES */
	struct gl_shape *square_hitbox_shape = load_mesh("meshes/square_hitbox.glMesh");
	struct gl_mesh *square_hitbox_mesh = init_mesh(square_hitbox_shape);
/* END LOAD MESH FOR HITBOXES */

/* LOAD TEXTURES */

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	stbi_set_flip_vertically_on_load(true);
	unsigned int cloud_tex = load_texture("textures/cloud.png");
/* END LOAD TEXTURES */

/* CREATE HITBOXES */
	struct gl_hitbox *square_hitbox = create_hitbox(square_hitbox_mesh);
	struct gl_hitbox *tall_square_hitbox = create_hitbox(square_hitbox_mesh);
	tall_square_hitbox->scale_y = 4.0f;
/* END CREATE HITBOXES */

/* CREATE OBJECTS WITH BUFFERS */
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

/* END CREATE OBJECTS */

	//Initialize VECTOR MODE
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

/* LOAD SHADERS START */
	// Vertex shader
	unsigned int vertexShader = compile_shader(V_SHADER_FILE, GL_VERTEX_SHADER);

	unsigned int fragmentShader = compile_shader(F_SHADER_FILE, GL_FRAGMENT_SHADER);

	unsigned int shaders[] = {vertexShader, fragmentShader};

	unsigned int shader1 = link_shaders(shaders, sizeof(shaders)/sizeof(shaders[0]));

	use_shader(shader1);	

/* LOAD SHADERS DONE */

/* */
	int glStatus = (int) glGetError();
	//fprintf(stdout, "Preloop %d\n", glStatus);
	
/* Pre-screen configuration */
	struct rgba bg_color = convert_to_rgba(0.0f, 0.1f, 0.1f, 1.0f);
	set_bg_color(bg_color);

	set_world_scale(WORLD_SCALE);
	set_cam_pos(0.0f, 0.0f);

	set_key_up(GLFW_KEY_F);
	set_key_down(GLFW_KEY_S);
	set_key_left(GLFW_KEY_R);
	set_key_right(GLFW_KEY_T);

/* MAIN LOOP START */
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
		if (check_collision_objects(player_object, tall_square))
			set_object_color(player_object, convert_to_rgba(0.7f, 0.0f, 0.3f, 0.8f));
		else
			set_object_color(player_object, convert_to_rgba(0.0f, 0.0f, 1.0f, 0.8f));


		glfwSwapBuffers(window);
		glfwPollEvents();    
		bind_array(0);
	}

/* MAIN LOOP DONE */

/* CLEAN UP */
	glDeleteProgram(shader1);

	destroy_mesh(square_mesh);
	destroy_mesh(triangle_mesh);

	destroy_object_list(bg_objects);
	destroy_object_list(mid_objects);
	destroy_object_list(fg_objects);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  
