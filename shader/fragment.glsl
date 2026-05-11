#version 330 core
out vec4 FragColor;

in vec2 frag_tex_coords;

uniform vec4 color;
uniform sampler2D texture1;
uniform bool useTexture;
uniform bool debuggingEnabled;


void main()
{
	if (useTexture){
		FragColor = texture(texture1, frag_tex_coords);
	}else{
		FragColor = color;
	}
}
