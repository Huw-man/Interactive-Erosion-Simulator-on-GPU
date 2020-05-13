#version 330 core

uniform vec3 u_cam_pos;
uniform vec3 u_light_intensity;
uniform vec3 u_light_dir;

uniform vec3 water_color;
uniform vec3 terrain_color;

in vec4 v_position;
in vec4 v_normal;
in vec2 v_uv;
in vec4 m_position;

uniform sampler2D tex1;
uniform sampler2D tex1_norms;
uniform sampler2D tex2;
uniform sampler2D tex2_norms;
uniform sampler2D tex3;
uniform sampler2D tex3_norms;

uniform vec2 texture_size;

uniform mat4 MV;

layout(location=0) out vec4 color;
layout(location=1) out vec4 normal;
layout(location=2) out vec4 pos;

#define M_PI 3.14159265358979323846

float rand(vec2 co){return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);}
float rand (vec2 co, float l) {return rand(vec2(rand(co), l));}
float rand (vec2 co, float l, float t) {return rand(vec2(rand(co, l), t));}

float perlin(vec2 p, float dim, float time) {
	vec2 pos = floor(p * dim);
	vec2 posx = pos + vec2(1.0, 0.0);
	vec2 posy = pos + vec2(0.0, 1.0);
	vec2 posxy = pos + vec2(1.0);
	
	float c = rand(pos, dim, time);
	float cx = rand(posx, dim, time);
	float cy = rand(posy, dim, time);
	float cxy = rand(posxy, dim, time);
	
	vec2 d = fract(p * dim);
	d = -0.5 * cos(d * M_PI) + 0.5;
	
	float ccx = mix(c, cx, d.x);
	float cycxy = mix(cy, cxy, d.x);
	float center = mix(ccx, cycxy, d.y);
	
	return center * 2.0 - 1.0;
}

float perlinFractal(vec2 m, float sd) {
	return   0.5333333 * perlin(m,1.0,0+sd)
				+ 0.2666667 * perlin(m,2.0,2.0+sd)
				+ 0.1333333 * perlin(m,4.0,6.72345+sd)
				+ 0.0666667 * perlin(m,8.0,3.123412341234+sd);
}

void main() {

	if (v_uv.x < 4.0/texture_size.x || v_uv.x > 1.0-4.0/texture_size.x || v_uv.y < 4.0/texture_size.y || v_uv.y > 1.0-4.0/texture_size.y) {
		discard;
	}

	vec3 ambient = vec3(1.0);
	float k_a = .5;
	vec3 out_ambient_3 = k_a * ambient;

	float rock_weight = pow(m_position.y*0.3,0.3)*1.0;
	float rock_sel = clamp(smoothstep(0.0,1.0,smoothstep(0.0,1.0,clamp((perlinFractal(v_uv*4.0,23.1) + 1.0 / 2.0),0.5,1.0)*rock_weight)),0,0.5);
	
	vec3 tex_color  = texture(tex1, v_uv*4.0).rgb;
	vec3 tex2_color  = texture(tex2, v_uv*8.0).rgb;
	vec3 tex3_color = texture(tex3, v_uv*64.0).rgb;
	float sel = (perlinFractal(v_uv*8.0,0.0) + 1.0) / 2.0;
	tex_color = mix(tex2_color, tex_color, clamp(sel+0.5,0,1));
	tex_color = mix(tex_color, tex3_color, rock_sel*0.2);

	// Can't normal map without tbn
	// vec3 tex_normal = (MV*vec4(texture(tex1_norms, v_uv*4.0).xyz,0)).xyz; 


	vec3 v_pos_3 = v_position.xyz;
	vec3 v_norm_3 = normalize(v_normal.xyz);
	float k_d = 1;
	// The 0.9 + / 1.9 is a slight hack to get slightly nicer ambient lighting
	vec3 out_diff_3 = k_d * u_light_intensity * max(0.0, (0.9+dot(v_norm_3, u_light_dir)) / 1.9 );



	vec3 v = u_cam_pos - v_pos_3;
	vec3 h = normalize(v + u_light_dir);
	float k_s = 0.0; //placeholder, specular constant for terrain
	
	int p = 48;

	vec3 out_spec_3 = k_s * u_light_intensity * pow(max(0.0, dot(v_norm_3, h)), p);

	color = vec4(out_ambient_3 + out_diff_3 + out_spec_3, 1) * vec4(tex_color, 1);
	normal = vec4(v_norm_3,1);
	pos = vec4(v_position.rgb/v_position.w,1);
}

