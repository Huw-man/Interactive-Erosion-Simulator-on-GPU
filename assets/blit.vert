#version 330 core

// often you might want vpos == uv
layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in vec2 vUV;

// the fragment shader can decide what to do with this UV
out vec2 UV;

void main(){
	gl_Position = vec4(vertexPosition_screenspace.xy, 0, 1);
	
	UV = vUV;
}