#version 330 core

layout(location = 0) in vec2 vertexPosition_screenspace;
layout(location = 2) in float source_intensity_vert;

out float source_intensity; // fragment source intensity
out vec2 UV;                // fragment UV

void main(){
	gl_Position = vec4(vertexPosition_screenspace, 0, 1);
	source_intensity = source_intensity_vert;
	UV = (vertexPosition_screenspace + 1.0)/2.0;
}