#version 330 core

// Just blits the data.

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 color;
uniform sampler2D tex;

void main() {
    color = texture( tex, UV);
}