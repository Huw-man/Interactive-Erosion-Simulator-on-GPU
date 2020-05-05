#version 330 core

// Just blits the data.

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;
uniform sampler2D water_colors;
uniform sampler2D water_normals;

void main() {
    color = texture(water_colors, UV);
}