#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "headers/shapes.h"
#include "headers/shaders.h"
#include "headers/buffers.h"
#include "headers/uniforms.h"
#include "headers/rgba.h"
#include "headers/meshes.h"

#define V_PI 3.1415

#define WORLD_SCALE 0.025

// Externs
extern struct rgba RGBA_BG_COLOR;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);


void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main()
{
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

	struct gl_shape *square_mesh = load_mesh("meshes/square.glMesh");	
	struct gl_shape *triangle_mesh = load_mesh("meshes/triangle.glMesh");	

/* END LOAD SHAPES */

/* CREATE BUFFERS START */
	// Square buffers
	struct buffer_t *VBO;
	struct buffer_t *EBO;
	unsigned int VAO;
	VBO = init_buffer(GL_ARRAY_BUFFER);
	EBO = init_buffer(GL_ELEMENT_ARRAY_BUFFER);

	// Data to be used
	struct buffer_data square_mesh_buf = pack_data(square_mesh->vertices, sizeof(float) * square_mesh->size_v, square_mesh->size_v, GL_FLOAT);
	struct buffer_data square_index_buf = pack_data(square_mesh->indices, sizeof(float) * square_mesh->size_i, square_mesh->size_i, GL_FLOAT);

	// Store the following actions into the VAO
	VAO = init_array();

	bind_buffer(VBO);
	//use glBufferData to pass the indices to vectors into the array

	// Element Buffer
	bind_buffer(EBO);
	
	// Pass vertices into the VBO
	set_buffer(VBO, square_mesh_buf, GL_DYNAMIC_DRAW);
	// Pass indices to the Element buffer
	set_buffer(EBO, square_index_buf, GL_DYNAMIC_DRAW);

	//layout 0, vec2, float values, normalize false, size of space between vertex data, offset
	set_array_attributes(0, 2, GL_FLOAT, sizeof(float)); 
//	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) 0);
//	glEnableVertexAttribArray(0);

	bind_array(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Triangle buffers
	struct buffer_t *VBO_T;
	struct buffer_t *EBO_T;
	unsigned int VAO_T;
	VBO_T = init_buffer(GL_ARRAY_BUFFER);
	EBO_T = init_buffer(GL_ELEMENT_ARRAY_BUFFER);

	struct buffer_data triangle_mesh_buf = pack_data(triangle_mesh->vertices, sizeof(float) * triangle_mesh->size_v, triangle_mesh->size_v, GL_FLOAT);
	struct buffer_data triangle_index_buf = pack_data(triangle_mesh->indices, sizeof(float) * triangle_mesh->size_i, triangle_mesh->size_i, GL_FLOAT);

	// Store the following actions into the VAO
	VAO_T = init_array();

	bind_buffer(VBO_T);
	//use glBufferData to pass the indices to vectors into the array

	// Element Buffer
	bind_buffer(EBO_T);
	
	// Pass vertices into the VBO
	set_buffer(VBO_T, triangle_mesh_buf, GL_DYNAMIC_DRAW);
	// Pass indices to the Element buffer
	set_buffer(EBO_T, triangle_index_buf, GL_DYNAMIC_DRAW);

	//layout 0, vec2, float values, normalize false, size of space between vertex data, offset
	set_array_attributes(0, 2, GL_FLOAT, sizeof(float)); 

	bind_array(0);


/* CREATE BUFFERS DONE */

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

/* MAIN LOOP START */
	while(!glfwWindowShouldClose(window))
	{
		processInput(window);

		// Clear the screen
		glClearColor(RGBA_BG_COLOR.r, RGBA_BG_COLOR.g, RGBA_BG_COLOR.b, RGBA_BG_COLOR.a);
		glClear(GL_COLOR_BUFFER_BIT);

		//glBindVertexArray(VAO);
		// Pass vertices into the VBO
//		glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_DYNAMIC_DRAW);
		// Pass indices to the Element buffer
//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_DYNAMIC_DRAW);

		// Use the vertex shader
		use_shader(shader1);

		// Set uniform values
		set_uniforms2(WORLD_SCALE, WORLD_SCALE, shader1, "screen_scalar");

		float rotation_val = glfwGetTime();
		set_uniforms1(rotation_val, shader1, "rotationRad");
		set_uniforms2(3.0f, 3.0f, shader1, "scalars");
		set_uniforms2(-2.0f, -4.0f, shader1, "translation");
		// Fragment uniforms
		set_uniforms4(1.0f, 0.0f, 0.0f, 1.0f, shader1, "color");

		// Bind the VAO
		// Draw a square
		bind_array(VAO);

		glDrawElements(GL_TRIANGLES, square_index_buf.elements, GL_UNSIGNED_INT, 0);

		rotation_val = V_PI/4.0f;
		set_uniforms1(rotation_val, shader1, "rotationRad");
		set_uniforms2(2.0f, 4.0f, shader1, "scalars");
		set_uniforms2(1.0f, 4.0f, shader1, "translation");

		// Fragment uniforms
		set_uniforms4(0.0f, 1.0f, 0.0f, 1.0f, shader1, "color");

		// Bind the triangle and draw
		bind_array(VAO_T);
		glDrawElements(GL_TRIANGLES, triangle_index_buf.elements, GL_UNSIGNED_INT, 0);
//		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();    
		bind_array(0);
	}

/* MAIN LOOP DONE */

/* CLEAN UP */
	glDeleteVertexArrays(1, &VAO);
	delete_buffer(VBO);
	delete_buffer(EBO);
	glDeleteProgram(shader1);

	destroy_gl_shape(square_mesh);
	destroy_gl_shape(triangle_mesh);

	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}  
