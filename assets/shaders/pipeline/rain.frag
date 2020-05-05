#version 330 core

// This uses blit.vert

in vec2 UV;

out vec4 color;


float mod289(float x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 mod289(vec4 x){return x - floor(x * (1.0 / 289.0)) * 289.0;}
vec4 perm(vec4 x){return mod289(((x * 34.0) + 1.0) * x);}

float noise(vec3 p){
    vec3 a = floor(p);
    vec3 d = p - a;
    d = d * d * (3.0 - 2.0 * d);

    vec4 b = a.xxyy + vec4(0.0, 1.0, 0.0, 1.0);
    vec4 k1 = perm(b.xyxy);
    vec4 k2 = perm(k1.xyxy + b.zzww);

    vec4 c = k2 + a.zzzz;
    vec4 k3 = perm(c);
    vec4 k4 = perm(c + 1.0);

    vec4 o1 = fract(k3 * (1.0 / 41.0));
    vec4 o2 = fract(k4 * (1.0 / 41.0));

    vec4 o3 = o2 * d.z + o1 * (1.0 - d.z);
    vec2 o4 = o3.yw * d.x + o3.xz * (1.0 - d.x);

    return o4.y * d.y + o4.x * (1.0 - d.y);
}

uniform float rain_intensity;
uniform int timestep;
uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s 
uniform float delta_t;
uniform vec2 texture_size;

void main() {
    //some arbitrary code to act as a river source, for debugging
    //to use, uncomment the block and comment out the line using noise
    float radius = 100;
    vec2 source_xy = vec2(0.75, 0.75) * texture_size;
    vec2 xy = UV * texture_size;
    if (length(source_xy - xy) <= radius) {
        color.y = texture(T1_bds,UV).y + 1.0*delta_t; 
    }
    else {
        color.y = texture(T1_bds,UV).y;
    }

    color.y = texture(T1_bds,UV).y + delta_t * clamp(noise(vec3(UV,float(timestep))),0.0,1.0) * rain_intensity; // add noise to old amount
	
    color.xzw = texture(T1_bds,UV).xzw; // passthrough
}