#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexColor;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 fragmentColor;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform sampler2D T1_bds;

void main() {
    vec3 vpos = vertexPosition_modelspace;
    vpos.y = texture(T1_bds, vertexUV).x;
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(vpos,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	fragmentColor = vertexColor;
}

