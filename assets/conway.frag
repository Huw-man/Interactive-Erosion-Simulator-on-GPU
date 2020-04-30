#version 330 core

// Performs a conway's game of life iteration on the uniform tex, making the next frame
// :'(



// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;

// the size of the screen. Or the framebuffer, in this case.
uniform vec2 screen_size;

uniform sampler2D tex;

void main() {
    const vec2 neighbor_offs[8] = vec2[8](
        vec2(-1,  0), vec2(1,  0), vec2(0, -1), vec2(0,  1),
        vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1)
    );
    float num_alive = 0;
    float my_sample = texture( tex, UV ).r;
    for (int i = 0; i < 8; ++i) {
        vec2 neighbor_p = UV + ((neighbor_offs[i]) / screen_size);
        num_alive += texture( tex, neighbor_p ).r;
    }

    if ( (my_sample > 0.5 && num_alive < 3.5 && num_alive > 1.5) || (num_alive < 3.5 && num_alive > 2.5)) {
        // survives
        color = vec4(1,1,1,1);
    }       
    else {
        // dies
        // (this is why we need social distancing)
        color = vec4(0,0,0,1);
    }
}