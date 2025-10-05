#ifndef KEYBINDS_H
#define KEYBINDS_H

#define MAX_SCANCODES 1024

int register_key_action_pair(int, void(*)(void));
void process_keybinds();

#endif
