#version 330 core

// Just blits the data.

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;
uniform sampler2D water_colors;
uniform sampler2D water_normals;
uniform sampler2D water_positions;

uniform sampler2D terrain_colors;
uniform sampler2D terrain_normals;
uniform sampler2D terrain_positions;
uniform sampler2D terrain_ao;

uniform samplerCube skybox;

uniform float zNear;
uniform float zFar;

uniform mat4 IVP;
uniform mat4 IV;
uniform mat4 IP;

uniform mat4 P;


float linearizeDepth(float z) {
    float z_n = 2.0 * z - 1.0;
    return 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
}

void main() {
    float w_depth = texture(water_positions, UV).z;
    float t_depth = texture(terrain_positions, UV).z;
    vec4 water_col = texture(water_colors, UV);
    vec4 terrain_col = texture(terrain_colors, UV)*texture(terrain_ao, UV).r;
    vec4 water_norm = texture(water_normals, UV);
    vec4 terrain_norm = texture(terrain_normals, UV);

    float ray_depth = w_depth - t_depth;

    if (terrain_col.a < 0.0001) {
        vec4 dir = IV * vec4( (IP*vec4(UV.xy*2.0-1.0, 1, 1)).xyz, 0);
        terrain_col = texture(skybox, dir.xyz);
    } 

    if (ray_depth > 0.00001) {
        float alph = (1.0 - exp(-ray_depth))*0.6+0.3*clamp(ray_depth*10000.0, 0, 1);


        color = mix(terrain_col, water_col, alph);
    }
    else {
        color = terrain_col;
    }
}
