float square[] = {
	1.0f, 1.0f, // Top right
	1.0f, -1.0f, // Bottom right
	-1.0f, -1.0f, // Bottom left
	-1.0f, 1.0f // Top left
};

unsigned int square_indices[] = {
	0, 1, 3,
	1, 2, 3
};

float triangle[] = {
	0.0f, 0.5f,    //top
	0.5f, 0.0f,     //right
	-0.5f, 0.0f   //left
};

unsigned int tri_indices[] = {
	0, 1, 2
};
