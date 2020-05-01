#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;

void main(){
	gl_Position = vec4(vertexPosition_screenspace.xy, 0, 1);
}