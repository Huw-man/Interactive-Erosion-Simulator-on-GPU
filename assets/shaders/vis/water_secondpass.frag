#version 330 core

// Just blits the data.

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;
uniform sampler2D water_colors;
uniform sampler2D water_normals;
uniform sampler2D water_depths;

uniform sampler2D terrain_colors;
uniform sampler2D terrain_normals;
uniform sampler2D terrain_depths;

uniform float zNear;
uniform float zFar;

float linearizeDepth(float z) {
    float z_n = 2.0 * z - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main() {
    float w_depth = texture(water_depths, UV).x;
    float t_depth = texture(terrain_depths, UV).x;
    vec4 water_col = texture(water_colors, UV);
    vec4 terrain_col = texture(terrain_colors, UV);
    vec4 water_norm = texture(water_normals, UV);
    vec4 terrain_norm = texture(terrain_normals, UV);

    float ray_depth = linearizeDepth(t_depth) - linearizeDepth(w_depth);

    if (ray_depth > 0.0001) {
        float alph = (1.0 - exp(-ray_depth))*0.8+0.2*smoothstep(0.0,0.01,ray_depth);
        color = mix(terrain_col, water_col, alph);
    }
    else {
        color = terrain_col;
    }
}