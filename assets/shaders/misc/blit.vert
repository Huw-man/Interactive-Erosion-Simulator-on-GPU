#version 330 core

// often you might want vpos == uv
layout(location = 0) in vec2 vertexPosition_screenspace;

// the fragment shader can decide what to do with this UV
out vec2 UV;

void main(){
	gl_Position = vec4(vertexPosition_screenspace, 0, 1);
	
	UV = (vertexPosition_screenspace + 1.0)/2.0;
}