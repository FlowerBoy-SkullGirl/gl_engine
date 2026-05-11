#include "headers/keybinds.h"
//#include <GL/glew.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

// The lists of key action pairs are stored in arrays because a typical input device will have no more than a few hundred distinct
// inputs, at most. Additional pairs are disallowed in the current version of the program
// This allows us to store the associated index of the keybind_actions[] array as a value in the keybind_list[] array
// Without compromising the time it takes to access or iterate through either list
void (*keybind_actions[MAX_SCANCODES])(void);
int keybind_list[MAX_SCANCODES] = {0};

//Shared with main
extern GLFWwindow* main_game_window;

int register_key_action_pair(int keycode, void(*action_func)(void))
{
	keybind_actions[keycode] = action_func;
	for (int i = 0; i < MAX_SCANCODES; i++){
		if(keybind_list[i] == 0){
			keybind_list[i] = keycode;
			return 0;
		}
	}
	return 1;
}

void process_keybinds()
{
	for (int i = 0; i < MAX_SCANCODES; i++){
		if(keybind_list[i] == 0)
			return;
		if (glfwGetKey(main_game_window, keybind_list[i]) == GLFW_PRESS)
			keybind_actions[keybind_list[i]]();
	}
}
