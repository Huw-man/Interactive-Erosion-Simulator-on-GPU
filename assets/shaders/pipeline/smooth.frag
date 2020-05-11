#version 330 core

// This uses blit.vert

in vec2 UV;

out vec4 color;

uniform sampler2D T1_bds; // T1_bds.x = b, T1_bds.y = d, T1_bds.z = s 
uniform vec2 texture_size;

uniform float m_hdiff;

// Heuristic to remove sharp peaks/valleys
// Since it seems that a non-smoothing approach simply won't work...
// Idea from https://github.com/karhu/terrain-erosion
void main() {
	vec4 self_bds = texture(T1_bds, UV);
    vec2 left_coord = vec2(UV.x - 1.0/texture_size.x, UV.y);
    vec4 left_bds = texture(T1_bds, left_coord);
    vec2 right_coord = vec2(UV.x + 1.0/texture_size.x, UV.y);
    vec4 right_bds = texture(T1_bds, right_coord);
    vec2 top_coord = vec2(UV.x, UV.y - 1.0/texture_size.y);
    vec4 top_bds = texture(T1_bds, top_coord);
    vec2 bot_coord = vec2(UV.x, UV.y + 1.0/texture_size.y);
    vec4 bot_bds = texture(T1_bds, bot_coord);

	float delta_h_L = self_bds.x - left_bds.x;
	float delta_h_R = self_bds.x - right_bds.x;
    
	float delta_h_T = self_bds.x - top_bds.x;
	float delta_h_B = self_bds.x - bot_bds.x;

    // Only do this at a minimum/maximum -- we can tell we're at a min/max if the sign of the derivative suddenly changes
    // In this case, it happens when they're the same, since the vectors are pointing in different directions
    float x_crv = delta_h_L * delta_h_R;
    float y_crv = delta_h_T * delta_h_B;

    color = self_bds;

    if ( ( (abs(delta_h_L) > m_hdiff || abs(delta_h_R) > m_hdiff) && x_crv > 0) || ( (abs(delta_h_T) > m_hdiff || abs(delta_h_B) > m_hdiff) && y_crv > 0) ) {
        color.x = (self_bds.x + left_bds.x + right_bds.x + top_bds.x + bot_bds.x) / 5.0; // Set height to average
    }
}