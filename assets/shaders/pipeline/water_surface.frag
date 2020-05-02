#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform sampler2D T2_f; // T2_f.x = f_L, T2_f.y = f_R, T2_f.z = f_T, t2_f.w = f_B

uniform vec2 texture_size; // gives the width/height of texture, to scale with uv coords accordingly

uniform float delta_t; //length of time steps

uniform vec2 l_xy; // l_xy.x = l_x, l_xy.y = l_y
// alternatively,
// uniform float l_x; // x dist between grid points
// uniform float l_y; // y dist between grid points

void main() {
	vec2 left_coord = vec2(UV.x - 1.0/texture_size.x, UV.y);
	vec4 f_from_left = texture(T2_f, left_coord);

	vec2 right_coord = vec2(UV.x + 1.0/texture_size.x, UV.y);
	vec4 f_from_right = texture(T2_f, right_coord);

	vec2 top_coord = vec2(UV.x, UV.y - 1.0/texture_size.y);
	vec4 f_from_top = texture(T2_f, top_coord);

	vec2 bot_coord = vec2(UV.x, UV.y + 1.0/texture_size.y);
	vec4 f_from_bot = texture(T2_f, bot_coord);

	vec4 f_out = texture(T2_f, UV);

	float delta_V = delta_t * (f_from_left.y + f_from_right.x + f_from_top.w + f_from_bot.z - f_out.x - f_out.y - f_out.z - f_out.w);

	vec4 self_bds = texture(T1_bds, UV);
	color.y = self_bds.y + delta_V / (l_xy.x * l_xy.y);
	color.xz = self_bds.xz; // passthrough
	color.w = self_bds.y + delta_V / (l_xy.x * l_xy.y) / 2;
}