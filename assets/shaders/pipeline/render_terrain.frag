#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 fragmentColor;

// Ouput data
out vec4 color;

uniform sampler2D T1_bds;
void main(){

	// Output color = color of the texture at the specified UV
	color = vec4(fragmentColor,1);
}