#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 1) in float source_intensity_vert;

out float source_intensity; // fragment source intensity

void main(){
	gl_Position = vec4(vertexPosition_screenspace.xy, 0, 1);
	source_intensity = source_intensity_vert
}