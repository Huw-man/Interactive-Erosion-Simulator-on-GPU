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
uniform float K_s_depthfac;
uniform float K_s_noisefac;
uniform sampler2D orig_T1;


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

float get_height_fac() {
	vec4 self_bds = texture(T1_bds, UV);
	vec4 orig_bds = texture(orig_T1, UV);
	return 1.0 + max(orig_bds.x - self_bds.x,0) * K_s_depthfac;
}

float get_noise_fac() { 
	return 1.0 + (perlinFractal(UV*64.0, 1.13254234)+1.0) * K_s_noisefac;
}

void main() {
	vec4 self_bds = texture(T1_bds, UV);
	vec2 vel = texture(T3_v, UV).xy;

	float sin_alpha = find_sin_alpha();

	// Additional constants (0.15) added by suggestion of the authors
	// We could also do the velocity a little differently, making the rain shader increase the outflow flux
	// But essentially the water will dissolve a tiny bit even if still
	float C = K.x * (sin_alpha + 0.15) * max(0.15,length(vel)) * clamp(self_bds.y,0,1);

	float fac = 1.0 / (get_height_fac() * get_noise_fac());

	if (C  > self_bds.z) {
		color.x = self_bds.x - K.y * (C - self_bds.z) * fac;
		color.z = self_bds.z + K.y * (C - self_bds.z) * fac;
	}
	else {
		color.x = self_bds.x + K.z * (self_bds.z - C);
		color.z = self_bds.z - K.z * (self_bds.z - C);
	}
	color.yw = self_bds.yw; // passthrough
}