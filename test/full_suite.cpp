#include <iostream>

//#include <GL/glew.h>
//#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "../src/headers/shapes.h"
#include "../src/headers/shaders.h"
#include "../src/headers/buffers.h"
#include "../src/headers/uniforms.h"
#include "../src/headers/rgba.h"
#include "../src/headers/meshes.h"
#include "../src/headers/textures.h"
#include "../src/headers/vectors.h"
#include "../src/headers/hitbox.h"
#include "../src/headers/hitbox_list.h"
#include "../src/headers/objects.h"
#include "../src/headers/camera.h"
#include "../src/headers/object_list.h"
#include "../src/headers/collision.h"
#include "../src/headers/keybinds.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../src/headers/stb_image.h"

// Testing libraries
#include <mcheck.h>

// Local globals
float g_time;
float g_delta_t;

GLFWwindow* main_game_window;

void mcheck_helper(enum mcheck_status status)
{
}

int object_memory_test()
{
	struct gl_shape *square_shape = load_mesh("meshes/square.glMesh");
	if (square_shape == NULL){
		printf("Error loading mesh from file\n");
		return 1;
	}

	struct gl_mesh *square_mesh = init_mesh(square_shape);
	if (square_mesh == NULL){
		printf("Error initializing mesh\n");
		return 1;
	}
	if (square_mesh->shape != square_shape){
		printf("Shape was not properly assigned to mesh\n");
		return 1;
	}

	build_buffers(square_mesh);
	if (square_mesh->VBO == NULL || square_mesh->EBO == NULL){
		printf("Error building buffers\n");
		return 1;
	}
	struct buffer_t *VBO = square_mesh->VBO;
	struct buffer_t *EBO = square_mesh->EBO;

	//Freeing the mesh should free all of its associated memory buffers and shapes
	destroy_mesh(square_mesh);

	int status = 0;
	status = mprobe(square_mesh);
/*
	if(status != MCHECK_FREE){
		printf("Error freeing mesh\n");
		printf("mprobe status: %d\n", status);
		return 1;
	}
	if(mprobe(square_shape) != MCHECK_OK || mprobe(VBO) != MCHECK_OK || mprobe(EBO) != MCHECK_OK){
		printf("Error freeing mesh members\n");
		return 1;
	}
*/
	
	return 0;
}

// GL callbacks
void glfw_error_callback(int error, const char* description)
{
	std::cout << description << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
	int tests_failed = 0;

	// Initialize mcheck debugging
	/*if(mcheck(NULL))
	{
		printf("mcheck failed to enable.\n");
	}
	printf("MCHECK_DISABLED: %d\n", MCHECK_DISABLED);
	printf("MCHECK_OK: %d\n", MCHECK_OK);
	printf("MCHECK_HEAD: %d\n", MCHECK_HEAD);
	printf("MCHECK_TAIL: %d\n", MCHECK_TAIL);
	printf("MCHECK_FREE: %d\n", MCHECK_FREE);
	*/
	// Initialize GL environment
	printf("Initializing GL environment\n");
	if(!glfwInit())
	{
		std::cout << "Failed glfw init" << std::endl;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwSetErrorCallback(glfw_error_callback);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Square", NULL, NULL);
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

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	printf("GL environment successfully initialized\n");


	// Run tests
	printf("Running object memory tests\n");
	if (object_memory_test())
		tests_failed++;

	if (!tests_failed)
		printf("All tests passed!\n");
	return tests_failed;
}
