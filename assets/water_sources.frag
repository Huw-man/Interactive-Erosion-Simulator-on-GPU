#version 330 core

in float source_intensity; // this is per-source
out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform float source_global_mult = 1.0;

int main() {
	color.y = T1_bds.y + source_intensity * source_global_mult; // add constant to old amount
	color.xzw = T1_bds.xzw; // passthrough
}