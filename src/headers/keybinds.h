#ifndef KEYBINDS_H
#define KEYBINDS_H

#define MAX_SCANCODES 1024

/* Keybinds are defined as a pair of data, the first value being
 * an integer that describes a GLFW_KEY and the second value being
 * a function pointer that takes no arguments
 * The process_keybinds() function then iterates a list of key-action-pairs
 * and executes the associated function if the key has been pressed
 */
int register_key_action_pair(int, void(*)(void));
void process_keybinds();

#endif
