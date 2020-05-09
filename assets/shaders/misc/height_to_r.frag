#version 330 core

in vec2 UV;
in float height_modelspace;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s

void main(){
	vec4 self_bds = texture(T1_bds, UV);

	color.x = height_modelspace / 2.0;
	color.yzw = self_bds.yzw;
}