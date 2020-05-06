#version 330 core

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

uniform vec3 water_color;
uniform vec3 terrain_color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

layout(location=0) out vec4 color;
layout(location=1) out vec4 normal;

void main() {
	vec3 ambient = vec3(0.4,0.4,0.4);
	float k_a = .5;
	vec3 out_ambient_3 = k_a * ambient;

	vec3 v_pos_3 = v_position.xyz;
	vec3 v_norm_3 = v_normal.xyz;
	vec3 l = u_light_pos - v_pos_3;
	float k_d = 1;
	vec3 out_diff_3 = k_d * u_light_intensity / (length(l) * length(l)) * max(0.0, (1.0 + dot(v_norm_3, normalize(l)))/2.0);

	vec3 v = u_cam_pos - v_pos_3;
	vec3 h = normalize(v + l);
	float k_s = .8; //placeholder, specular constant for water
	
	int p = 48;

	vec3 out_spec_3 = k_s * u_light_intensity / (length(l) * length(l)) * pow(max(0.0, dot(v_norm_3, h)), p);

	color = vec4(out_ambient_3 + out_diff_3 + out_spec_3, 1) * vec4(water_color, 1);
	normal = v_normal;
}
