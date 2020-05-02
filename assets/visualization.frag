#version 330 core

uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

uniform vec4 water_color;
uniform vec4 terrain_color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

out vec4 out_color;

int main() {
	vec4 self_bds = texture(T1_bds, v_uv);
	float water_prop = self_bds.y / (self_bds.x + self_bds.y);
	vec4 color = water_prop * water_color + (1 - water_prop) * terrain_color;

	vec3 ambient = vec3(1,1,1);
	float k_a = .1;
	vec3 out_ambient_3 = k_a * ambient;

	vec3 v_pos_3 = v_position.xyz;
	vec3 v_norm_3 = v_normal.xyz;
	vec3 l = u_light_pos - v_pos_3;
	float k_d = 1;
	vec3 out_diff_3 = k_d * u_light_intensity / (length(l) * length(l)) * max(0.0, dot(v_norm_3, normalize(l)));

	vec3 v = u_cam_pos - v_pos_3;
	vec3 h = normalize(v + l);
	if (self_bds.d == 0) { //no water
		float k_s = .2; //placeholder, specular constant for terrain
	}
	else {
		float k_s = .8; //placeholder, specular constant for water
	}
	
	int p = 48;

	vec3 out_spec_3 = k_s * u_light_intensity / (length(l) * length(l)) * pow(max(0.0, dot(v_norm_3, h)), p);

	out_color = vec4(out_ambient_3 + out_diff_3 + out_spec_3, 1) * color;
}

