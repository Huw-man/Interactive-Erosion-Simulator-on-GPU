#version 330 core

in vec2 UV;
in float alpha; //angle of the terrain in radians

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform sampler2D T3_v; // T3_v.x = u (x component of vel), T3_v.y = v (y component of vel)

uniform vec3 K; //K.x = K_c, K.y = K_s, K.z = K_d
//alternatively,
//uniform float K_c; //sediment capacity constant
//uniform float K_s; //dissolving constant
//uniform float K_d; //deposition constant

int main() {
	vec2 vel = texture(T3_v, UV).xy;
	float C = K.x * sin(alpha) * length(vel);

	vec4 self_bds = texture(T1_bds, UV);
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