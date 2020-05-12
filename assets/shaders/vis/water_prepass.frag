#version 330 core

uniform vec3 u_cam_pos;
uniform vec3 u_light_dir;
uniform vec3 u_light_intensity;

uniform vec3 water_color;
uniform vec3 terrain_color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;

uniform vec2 texture_size;

layout(location=0) out vec4 color;
layout(location=1) out vec4 normal;
layout(location=2) out vec4 pos;

void main() {

	if (v_uv.x < 4.0/texture_size.x || v_uv.x > 1.0-4.0/texture_size.x || v_uv.y < 4.0/texture_size.y || v_uv.y > 1.0-4.0/texture_size.y) {
		discard;
	}

	vec4 self_bds = texture(T1_bds, v_uv);

	vec3 ambient = vec3(0.4,0.4,0.4);
	float k_a = .5;
	vec3 out_ambient_3 = k_a * ambient;

	vec3 v_pos_3 = v_position.xyz;
	vec3 v_norm_3 = normalize(v_normal.xyz);
	float k_d = 1;
	// The 0.9 + / 1.8 is a slight hack to get slightly nicer ambient lighting
	vec3 out_diff_3 = k_d * u_light_intensity * max(0.0, (0.9+dot(v_norm_3, u_light_dir)) / 1.9 );



	vec3 v = u_cam_pos - v_pos_3;
	vec3 h = normalize(v + u_light_dir);
	float k_s = 0.8; //placeholder, specular constant for terrain
	
	int p = 48;

	vec3 out_spec_3 = k_s * u_light_intensity * pow(max(0.0, dot(v_norm_3, h)), p);

	color = vec4(out_ambient_3 + out_diff_3 + out_spec_3, 1) * vec4(water_color, 1);

	// Toggle this on/off to highlight sediment-carrying water
	color.rgb = mix(color.rgb, vec3(0.5,0.24,0.177), clamp(self_bds.b*150,0,0.4));

	normal = vec4(v_norm_3,1);
	pos = vec4(v_position.rgb,1);
}

