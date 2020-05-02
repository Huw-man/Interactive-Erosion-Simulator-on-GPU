#version 330 core

// Credit:
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83

// Gives white noise

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

uniform vec2 screen_size;

float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

void main() {
    float n = noise(UV*screen_size);

    bool discrete = true;
    float threshold = 0.4;
    if (discrete) {
        n = step(threshold,n);
    }

    color = vec4(n,n,n,1);
}