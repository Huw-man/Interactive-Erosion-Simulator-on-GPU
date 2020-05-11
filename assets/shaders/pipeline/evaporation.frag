#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform float K_e; // evaporation constant
uniform float delta_t;

void main(){
	vec4 self_bds = texture(T1_bds, UV);

	color.y = max(self_bds.y * (1 - K_e * delta_t),0);
	color.xzw = self_bds.xzw; // passthrough
}