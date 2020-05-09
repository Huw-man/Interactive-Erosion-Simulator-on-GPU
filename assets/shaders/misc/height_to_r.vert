#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out float height_modelspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){
    vec2 uv_c = vec2(vertexUV.x-0.5,vertexUV.y+0.5)*2.0;
    
	gl_Position = vec4(uv_c, 0.01 ,1);
	
	// UV of the vertex. No special space for this one.
	UV = vertexUV;
	height_modelspace = vertexPosition_modelspace.y;
}

