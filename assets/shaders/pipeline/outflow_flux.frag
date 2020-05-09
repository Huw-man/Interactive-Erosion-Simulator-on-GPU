#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform sampler2D T2_f; // T2_f.x = f_L, T2_f.y = f_R, T2_f.z = f_T, t2_f.w = f_B

uniform vec2 texture_size; // gives the width/height of texture, to scale with uv coords accordingly

uniform float delta_t; //length of time steps

uniform vec3 alg; //alg.x = A, alg.y = l, alg.z = g
// alternatively,
// uniform float A; //cross sectional area of pipes
// uniform float l; //length of pipes
// uniform float g; //gravity

uniform vec2 l_xy; // l_xy.x = l_x, l_xy.y = l_y
// alternatively,
// uniform float l_x; // x dist between grid points
// uniform float l_y; // y dist between grid points

void main() {
	vec4 self_bds = texture(T1_bds, UV);
	vec4 f = texture(T2_f, UV);
	if (UV.x < 1.0/texture_size.x) { // on the left boundary
		color.x = 0;
	}
	else {
		vec2 left_coord = vec2(UV.x - 1.0/texture_size.x, UV.y);
		vec4 left_bds = texture(T1_bds, left_coord);
		float delta_h_L = self_bds.x + self_bds.y - left_bds.x - left_bds.y;

		float f_L_next = max(0, f.x + delta_t * alg.x * alg.z * delta_h_L / alg.y);

		color.x = f_L_next;
	}
	if (UV.x >  1 - (1.0/texture_size.x)) { // on the right boundary
		color.y = 0;
	}
	else {
		vec2 right_coord = vec2(UV.x + 1.0/texture_size.x, UV.y);
		vec4 right_bds = texture(T1_bds, right_coord);
		float delta_h_R = self_bds.x + self_bds.y - right_bds.x - right_bds.y;

		float f_R_next = max(0, f.y + delta_t * alg.x * alg.z * delta_h_R / alg.y);
		
		color.y = f_R_next;
	}
	if (UV.y < (1.0/texture_size.y)) { // on the top boundary
		color.z = 0;
	}
	else {
		vec2 top_coord = vec2(UV.x, UV.y - 1.0/texture_size.y);
		vec4 top_bds = texture(T1_bds, top_coord);
		float delta_h_T = self_bds.x + self_bds.y - top_bds.x - top_bds.y;

		float f_T_next = max(0, f.z + delta_t * alg.x * alg.z * delta_h_T / alg.y);
		
		color.z = f_T_next;
	} 
	if (UV.y > 1 - (1.0/texture_size.y)) { // on the bottom boundary
		color.w = 0;
	}
	else {
		vec2 bot_coord = vec2(UV.x, UV.y + 1.0/texture_size.y);
		vec4 bot_bds = texture(T1_bds, bot_coord);
		float delta_h_B = self_bds.x + self_bds.y - bot_bds.x - bot_bds.y;

		float f_B_next = max(0, f.w + delta_t * alg.x * alg.z * delta_h_B / alg.y);
		
		color.w = f_B_next;
	}

	float K = min(self_bds.y * l_xy.x * l_xy.y / (color.x + color.y + color.z + color.w) / delta_t, 1.0);
	color = K * color;
}