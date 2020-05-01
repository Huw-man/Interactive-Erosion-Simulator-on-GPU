#version 330 core

in float source_intensity; // this is per-source
in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform float source_global_mult = 1.0;
uniform float delta_t;

int main() {
	color.y = texture(T1_bds,UV).y + delta_t * source_intensity * source_global_mult; // add constant to old amount
	color.xzw = texture(T1_bds,UV).xzw; // passthrough
}