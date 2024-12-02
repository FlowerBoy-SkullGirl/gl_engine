#include "headers/keybinds.h"

int keybind_up;
int keybind_down;
int keybind_left;
int keybind_right;

void set_key_up(int k)
{
	keybind_up = k;
}

void set_key_down(int k)
{
	keybind_down = k;
}

void set_key_left(int k)
{
	keybind_left = k;
}

void set_key_right(int k)
{
	keybind_right = k;
}
