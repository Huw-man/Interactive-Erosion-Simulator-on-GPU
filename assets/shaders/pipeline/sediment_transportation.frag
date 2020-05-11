#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s
uniform sampler2D T3_v; // T3_v.x = u (x component of vel), T3_v.y = v (y component of vel)

uniform vec2 texture_size; // gives the width/height of texture, to scale with uv coords accordingly

uniform float delta_t;

void main(){
	vec4 self_bds = texture(T1_bds, UV);
	vec2 vel = texture(T3_v, UV).xy;
	//scale up to xy coords so we can apply the vel*delta_t step
	vec2 XY = UV * texture_size;
	float x_step = clamp(XY.x - vel.x * delta_t, 0, texture_size.x);
	float y_step = clamp(XY.y - vel.y * delta_t, 0, texture_size.y);

	//remember to divide back down to sample with UV coords
	float s_top_left = texture(T1_bds, vec2(floor(x_step) / texture_size.x, floor(y_step) / texture_size.y)).z;
	float s_top_right = texture(T1_bds, vec2(ceil(x_step) / texture_size.x, floor(y_step) / texture_size.y)).z;
	float s_bot_left = texture(T1_bds, vec2(floor(x_step) / texture_size.x, ceil(y_step) / texture_size.y)).z;
	float s_bot_right = texture(T1_bds, vec2(ceil(x_step) / texture_size.x, ceil(y_step) / texture_size.y)).z;

	//first interp
	float s_top = (x_step - floor(x_step)) * s_top_right + (ceil(x_step) - x_step) * s_top_left;
	float s_bot = (x_step - floor(x_step)) * s_bot_right + (ceil(x_step) - x_step) * s_bot_left;

	//second interp for final val
	color.z = (y_step - floor(y_step)) * s_top + (ceil(y_step) - y_step) * s_bot;
	
	color.xyw = self_bds.xyw; // passthrough
}