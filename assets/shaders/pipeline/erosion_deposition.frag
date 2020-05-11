#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform sampler2D T3_v; // T3_v.x = u (x component of vel), T3_v.y = v (y component of vel)

uniform vec3 K; //K.x = K_c, K.y = K_s, K.z = K_d
uniform vec2 l_xy; // l_xy.x = l_x, l_xy.y = l_y
//alternatively,
//uniform float K_c; //sediment capacity constant
//uniform float K_s; //dissolving constant
//uniform float K_d; //deposition constant
uniform vec2 texture_size;

float find_sin_alpha() {
	float self_b = texture(T1_bds, UV).x;
	float r_b = texture(T1_bds, UV + vec2(1.0 / texture_size.x, 0)).x;
	float l_b = texture(T1_bds, UV - vec2(1.0 / texture_size.x, 0)).x;
	float d_b = texture(T1_bds, UV + vec2(0, 1.0 / texture_size.y)).x;
	float u_b = texture(T1_bds, UV - vec2(0, 1.0 / texture_size.y)).x;

	float dbdx = (r_b-l_b) / (2.0*l_xy.x/texture_size.x);
	float dbdy = (r_b-l_b) / (2.0*l_xy.y/texture_size.y);

	return sqrt(dbdx*dbdx+dbdy*dbdy)/sqrt(1+dbdx*dbdx+dbdy*dbdy);
}

vec4 avg(sampler2D tex) {
	vec4 a = texture(tex, UV + texture_size*vec2(1,0));
	vec4 b = texture(tex, UV + texture_size*vec2(-1,0));
	vec4 c = texture(tex, UV + texture_size*vec2(0,1));
	vec4 d = texture(tex, UV + texture_size*vec2(0,-1));
	vec4 e = texture(tex, UV);
	return (a+b+c+d+e)/5.0;
}

void main() {
	vec4 self_bds = texture(T1_bds, UV);
	vec2 vel = texture(T3_v, UV).xy;

	float sin_alpha = find_sin_alpha();

	float C = K.x * sin_alpha * length(vel) * clamp(self_bds.y,0,1);

	if (C > self_bds.z) {
		color.x = self_bds.x - K.y * (C - self_bds.z);
		color.z = self_bds.z + K.y * (C - self_bds.z);
	}
	else {
		color.x = self_bds.x + K.z * (self_bds.z - C);
		color.z = self_bds.z - K.z * (self_bds.z - C);
	}
	color.yw = self_bds.yw; // passthrough
}